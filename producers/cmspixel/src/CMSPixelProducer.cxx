#include "eudaq/Producer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/RawDataEvent.hh"
#include "eudaq/Timer.hh"
#include "eudaq/Utils.hh"
#include "eudaq/OptionParser.hh"
#include "eudaq/Configuration.hh"
#include "config.h" // Version symbols

#include "api.h"
#include "constants.h"
#include "dictionaries.h"
#include "log.h"
#include "helper.h"

#include "CMSPixelProducer.hh"

#include <iostream>
#include <ostream>
#include <vector>
#include <sched.h>

using namespace pxar;
using namespace std;

// event type name, needed for readout with eudaq. Links to /main/lib/src/CMSPixelConverterPlugin.cc:
static const std::string EVENT_TYPE_DUT = "CMSPixelDUT";
static const std::string EVENT_TYPE_REF = "CMSPixelREF";
static const std::string EVENT_TYPE_TRP = "CMSPixelTRP";
static const std::string EVENT_TYPE_ANA = "CMSPixelANA";
static const std::string EVENT_TYPE_DIG = "CMSPixelDIG";



CMSPixelProducer::CMSPixelProducer(const std::string & name, const std::string & runcontrol, const std::string & verbosity)
  : eudaq::Producer(name, runcontrol),
    m_run(0),
    m_ev(0),
    m_ev_filled(0),
    m_ev_runningavg_filled(0),
    m_tlu_waiting_time(4000),
    m_roc_resetperiod(0),
    m_nplanes(1),
    m_terminated(false),
    m_running(false),
    m_api(NULL),
    m_verbosity(verbosity),
    m_trimmingFromConf(false),
    m_pattern_delay(0),
    m_trigger_is_pg(false),
    m_fout(0),
    m_foutName(""),
    triggering(false),
    m_roctype(""),
    m_pcbtype(""),
    m_usbId(""),
    m_producerName(name),
    m_detector(""),
    m_event_type(""),
    m_alldacs(""),
    m_last_mask_filename("")
{
  if(m_producerName.find("REF") != std::string::npos) {
    m_detector = "REF";
    m_event_type = EVENT_TYPE_REF;
  }
  else if(m_producerName.find("TRP") != std::string::npos) {
    m_detector = "TRP";
    m_event_type = EVENT_TYPE_TRP;

  }
  else if(m_producerName.find("ANA") != std::string::npos) {
    m_detector = "ANA";
    m_event_type = EVENT_TYPE_ANA;
  }
  else if(m_producerName.find("DIG") != std::string::npos) {
    m_detector = "DIG";
    m_event_type = EVENT_TYPE_DIG;
  }
  else {
    m_detector = "DUT";
    m_event_type = EVENT_TYPE_DUT;
  }
}

void CMSPixelProducer::OnConfigure(const eudaq::Configuration & config) {

  std::cout << "Configuring: " << config.Name() << std::endl;
  m_config = config;
  bool confTrimming(false), confDacs(false);
  // declare config vectors
  std::vector<std::pair<std::string,uint8_t> > sig_delays;
  std::vector<std::pair<std::string,double> > power_settings;
  std::vector<std::pair<std::string,uint8_t> > pg_setup;
  std::vector<std::vector<std::pair<std::string,uint8_t> > > tbmDACs;
  std::vector<std::vector<std::pair<std::string,uint8_t> > > rocDACs;
  std::vector<std::vector<pxar::pixelConfig> > rocPixels;
  std::vector<masking> maskPixels;
  std::vector<uint8_t> rocI2C;

  uint8_t hubid = config.Get("hubid", 31);
  std::string value = config.Get("event_type","INVALID");
  std::transform(value.begin(), value.end(), value.begin(), ::tolower);
  if (value.find("dut") != std::string::npos)
          m_event_type = EVENT_TYPE_DUT;
  else if (value.find("ref") != std::string::npos)
          m_event_type = EVENT_TYPE_REF;
  else if (value.find("trp") != std::string::npos)
          m_event_type = EVENT_TYPE_TRP;
  else if (value.find("analog") != std::string::npos)
          m_event_type = EVENT_TYPE_ANA;
  else if (value.find("digital") != std::string::npos)
          m_event_type = EVENT_TYPE_DIG;
  EUDAQ_INFO(string("Set Event Type of " + m_producerName + " to " + m_event_type + "."));
  value = config.Get("detector_name","INVALID");
  if (value != "INVALID"){
    m_detector = value;
    EUDAQ_INFO(string("Set Detector Name to \"" + (string) m_detector + "\"."));
  }

  // Store waiting time in ms before the DAQ is stopped in OnRunStop():
  m_tlu_waiting_time = config.Get("tlu_waiting_time", 4000);
  EUDAQ_INFO(string("Waiting " + std::to_string(m_tlu_waiting_time) + "ms before stopping DAQ after run stop."));

  // DTB delays
  sig_delays.push_back(std::make_pair("clk",config.Get("clk",4)));
  sig_delays.push_back(std::make_pair("ctr",config.Get("ctr",4)));
  sig_delays.push_back(std::make_pair("sda",config.Get("sda",19)));
  sig_delays.push_back(std::make_pair("tin",config.Get("tin",9)));
  sig_delays.push_back(std::make_pair("deser160phase",config.Get("deser160phase",4)));
  sig_delays.push_back(std::make_pair("level",config.Get("level",15)));
  sig_delays.push_back(std::make_pair("triggerlatency",config.Get("triggerlatency",86)));
  sig_delays.push_back(std::make_pair("tindelay",config.Get("tindelay",13)));
  sig_delays.push_back(std::make_pair("toutdelay",config.Get("toutdelay",10)));
  sig_delays.push_back(std::make_pair("triggertimeout",config.Get("triggertimeout",65000)));
  //Power settings:
  power_settings.push_back( std::make_pair("va",config.Get("va",1.8)) );
  power_settings.push_back( std::make_pair("vd",config.Get("vd",2.5)) );
  power_settings.push_back( std::make_pair("ia",config.Get("ia",1.10)) );
  power_settings.push_back( std::make_pair("id",config.Get("id",1.10)) );

  // Periodic ROC resets:
  m_roc_resetperiod = config.Get("rocresetperiod", 0);
  if(m_roc_resetperiod > 0) { EUDAQ_USER("Sending periodic ROC resets every " + eudaq::to_string(m_roc_resetperiod) + "ms\n"); }

  // Pattern Generator:
  bool testpulses = config.Get("testpulses", false);
  if(testpulses) {
    pg_setup.push_back(std::make_pair("resetroc",config.Get("resetroc",25)) );
    pg_setup.push_back(std::make_pair("calibrate",config.Get("calibrate",106)) );
    pg_setup.push_back(std::make_pair("trigger",config.Get("trigger", 16)) );
    pg_setup.push_back(std::make_pair("token",config.Get("token", 0)));
    m_pattern_delay = config.Get("patternDelay", 100) * 10;
    EUDAQ_USER("Using testpulses, pattern delay " + eudaq::to_string(m_pattern_delay) + "\n");
  }
  else {
    pg_setup.push_back(std::make_pair("trigger",46));
    pg_setup.push_back(std::make_pair("token",0));
    m_pattern_delay = config.Get("patternDelay", 100);
  }

  try {
    // Check for multiple ROCs using the I2C parameter:
    std::vector<int32_t> i2c_addresses = split(config.Get("i2c","i2caddresses","-1"),' ');
    std::cout << "Found " << i2c_addresses.size() << " I2C addresses: " << pxar::listVector(i2c_addresses) << std::endl;

    // Set the type of the TBM and read registers if any:
    m_tbmtype = config.Get("tbmtype","notbm");
    try {
      tbmDACs.push_back(GetConfDACs(0,true));
      tbmDACs.push_back(GetConfDACs(1,true));
    }
    catch(pxar::InvalidConfig) {}

    // Set the type of the ROC correctly:
    m_roctype = config.Get("roctype","psi46digv21respin");

    // Read the type of carrier PCB used ("desytb", "desytb-rot"):
    m_pcbtype = config.Get("pcbtype","desytb");

    // Read the mask file if existent:
    std::vector<pxar::pixelConfig> maskbits = GetConfMaskBits();

    // Read DACs and Trim settings for all ROCs, one for each I2C address:
    for(int32_t i2c : i2c_addresses) {
      // Read trim bits from config:
      rocPixels.push_back(GetConfTrimming(maskbits,static_cast<int16_t>(i2c)));
      // Read the DAC file and update the vector with overwrite DAC settings from config:
      rocDACs.push_back(GetConfDACs(static_cast<int16_t>(i2c)));
      // Add the I2C address to the vector:
      if(i2c > -1) { rocI2C.push_back(static_cast<uint8_t>(i2c)); }
      else { rocI2C.push_back(static_cast<uint8_t>(0)); }
    }

//<<<<<<< HEAD
//=======

    // Set the type of the ROC correctly:
    m_roctype = config.Get("roctype","psi46digv2");

    // Read the type of carrier PCB used ("desytb", "desytb-rot"):
    m_pcbtype = config.Get("pcbtype","desytb");
//>>>>>>> e3909fa378d09c097e867dbeacb5a1f75eb82687

    // create api
    if(m_api != NULL) { delete m_api; }

    m_usbId = config.Get("usbId","*");
    EUDAQ_EXTRA("Trying to connect to USB id: " + m_usbId + "\n");

    // Allow overwriting of verbosity level via command line:
    m_verbosity = config.Get("verbosity", m_verbosity);

    // Get a new pxar instance:
    m_api = new pxar::pxarCore(m_usbId, m_verbosity);

    // Initialize the testboard:
    if(!m_api->initTestboard(sig_delays, power_settings, pg_setup)) {
      EUDAQ_ERROR(string("Firmware mismatch."));
      throw pxar::pxarException("Firmware mismatch");
    }

    // Initialize the DUT as configured above:
    m_api->initDUT(hubid,m_tbmtype,tbmDACs,m_roctype,rocDACs,rocPixels,rocI2C);
    // Store the number of configured ROCs to be stored in a BORE tag:
    m_nplanes = rocDACs.size();

    // Read current:
    std::cout << "Analog current: " << m_api->getTBia()*1000 << "mA" << std::endl;
    std::cout << "Digital current: " << m_api->getTBid()*1000 << "mA" << std::endl;

    if(m_api->getTBid()*1000 < 15) EUDAQ_ERROR(string("Digital current too low: " + std::to_string(1000*m_api->getTBid()) + "mA"));
    else EUDAQ_INFO(string("Digital current: " + std::to_string(1000*m_api->getTBid()) + "mA"));

    if(m_api->getTBia()*1000 < 15) EUDAQ_ERROR(string("Analog current too low: " + std::to_string(1000*m_api->getTBia()) + "mA"));
    else EUDAQ_INFO(string("Analog current: " + std::to_string(1000*m_api->getTBia()) + "mA"));

    // Send a single RESET to the ROC to initialize its status:
    if(!m_api->daqSingleSignal("resetroc")) { throw InvalidConfig("Unable to send ROC reset signal!"); }

    // Switching to external clock if requested and check if DTB returns TRUE status:
    std::cout << "external clock: " << config.Get("extclock", 1) << std::endl;
    if(!m_api->setExternalClock(config.Get("extclock",1) != 0 ? true : false)) {
      throw InvalidConfig("Couldn't switch to " + string(config.Get("extclock",1) != 0 ? "external" : "internal") + " clock.");
    }
    else {
      EUDAQ_INFO(string("Clock set to " + string(config.Get("extclock",1) != 0 ? "external" : "internal")));
    }

    // Switching to the selected trigger source and check if DTB returns TRUE:
    std::string triggersrc = config.Get("trigger_source","extern");
    if(!m_api->daqTriggerSource(triggersrc)) {
      throw InvalidConfig("Couldn't select trigger source " + string(triggersrc));
    }
    else {
      // Update the TBM setting according to the selected trigger source.
      // Switches to TBM_EMU if we selected a trigger source using the TBM EMU.
      TriggerDictionary* trgDict;
      if(m_tbmtype == "notbm" && trgDict->getInstance()->getEmulationState(triggersrc)) {
	m_tbmtype = "tbmemulator";
      }

      if(triggersrc == "pg" || triggersrc == "pg_dir" || triggersrc == "patterngenerator") {
	m_trigger_is_pg = true;
      }
      EUDAQ_INFO(string("Trigger source selected: " + triggersrc));
    }

    // Output the configured signal to the probes:
    std::string signal_d1 = config.Get("signalprobe_d1","off");
    std::string signal_d2 = config.Get("signalprobe_d2","off");
    std::string signal_a1 = config.Get("signalprobe_a1","off");
    std::string signal_a2 = config.Get("signalprobe_a2","off");

    if(m_api->SignalProbe("d1", signal_d1) && signal_d1 != "off") {
      EUDAQ_EXTRA("Setting scope output D1 to \"" + signal_d1 + "\"\n");
    }
    if(m_api->SignalProbe("d2", signal_d2) && signal_d2 != "off") {
      EUDAQ_EXTRA("Setting scope output D2 to \"" + signal_d2 + "\"\n");
    }
    if(m_api->SignalProbe("a1", signal_a1) && signal_a1 != "off") {
      EUDAQ_EXTRA("Setting scope output A1 to \"" + signal_a1 + "\"\n");
    }
    if(m_api->SignalProbe("a2", signal_a2) && signal_a2 != "off") {
      EUDAQ_EXTRA("Setting scope output A2 to \"" + signal_a2 + "\"\n");
    }

    EUDAQ_EXTRA(m_api->getVersion() + string(" API set up successfully...\n"));

    // test pixels
    if(testpulses) {
      std::cout << "Setting up pixels for calibrate pulses..." << std::endl << "col \t row" << std::endl;
      for(int i = 40; i < 45; i++){
	m_api->_dut->testPixel(25,i,true);
      }

    }

    /**MASKING*/

    maskPixels = GetConfMask();
    //vector<vector<bool> > maskArray; maskArray.resize(52); for (uint16_t row(0); row<52; row++) maskArray[row].resize(80);
    string hexMask;

    /**mask whole ROC for the antimask identifier*/
    for (uint16_t i(0); i<maskPixels.size(); i++){
        if (maskPixels[i].id()[0] != '#' && maskPixels[i].id()[0] != ' ' && maskPixels[i].id().size()>0){
            uint16_t roc = maskPixels[i].roc();
            if (maskPixels[i].id() == "cornBot") {
                m_api->_dut->maskAllPixels(true,roc);
                cout << "masking whole ROC " << roc << "\n";
            }
        }}

    for (uint16_t i(0); i<maskPixels.size(); i++){
        if (maskPixels[i].id()[0] != '#' && maskPixels[i].id()[0] != ' ' && maskPixels[i].id().size()>0){
            uint16_t col = maskPixels[i].col(); string cCol = to_string(col/16)+char(col%16 < 10 ? col%16+'0' : col%16+'W');
            uint16_t row = maskPixels[i].row(); string cRow = to_string(row/16)+char(row%16 < 10 ? row%16+'0' : row%16+'W');
            uint16_t roc = maskPixels[i].roc(); char cRoc = (roc < 10 ? roc+'0' : roc+'W');
            string sCol = cRoc+cCol, sRow = "00"+cRow;

            if (maskPixels[i].id() == "pix"){
                m_api->_dut->maskPixel(col, row, true, roc);
                //maskArray[col][row] = true;
                hexMask.insert(0," ").insert(0,sRow).insert(0," ").insert(0,sCol).insert(0,"a");}
            else if (maskPixels[i].id() == "col"){
                hexMask.insert(0," ").insert(0,sRow).insert(0," ").insert(0,sCol).insert(0,"b");
                for (uint16_t nCols(col); nCols < row+1; nCols++){
                    for (uint16_t nRows(0); nRows < 80; nRows++) {
                        m_api->_dut->maskPixel(nCols, nRows, true, roc);
                        //maskArray[nCols][nRows] = true;
                        }}} //end elseif
            else if (maskPixels[i].id() == "row"){
                hexMask.insert(0," ").insert(0,sRow).insert(0," ").insert(0,sCol).insert(0,"c");
                for (uint16_t nRows(col); nRows < row+1; nRows++){
                    for (uint16_t nCols(0); nCols < 52; nCols++) {
                        m_api->_dut->maskPixel(nCols, nRows, true, roc);
                        //maskArray[nCols][nRows] = true;
                        }}} //end elseif
            else if (maskPixels[i].id() == "roc"){
                hexMask.insert(0," ").insert(0,sRow).insert(0," ").insert(0,sCol).insert(0,"d");
                cout << "masked roc "<< roc <<"\n";
                EUDAQ_INFO(string("ROC ") + string(to_string(roc)) + string(" fully masked"));
                m_api->_dut->maskAllPixels(true,roc);} //end elseif

            /**antimask*/
            else if (maskPixels[i].id() == "cornBot" && maskPixels[i+1].id() == "cornTop"){
                uint16_t row_start = maskPixels[i].row();   string cRow1 = to_string(row_start/16)+char(row_start%16 < 10 ? row_start%16+'0' : row_start%16+'W');
                uint16_t row_stop = maskPixels[i+1].row();  string cRow2 = to_string(row_stop/16)+char(row_stop%16 < 10 ? row_stop%16+'0' : row_stop%16+'W');
                uint16_t col_start = maskPixels[i].col();   string cCol1 = to_string(col_start/16)+char(col_start%16 < 10 ? col_start%16+'0' : col_start%16+'W');
                uint16_t col_stop = maskPixels[i+1].col();  string cCol2 = to_string(col_stop/16)+char(col_stop%16 < 10 ? col_stop%16+'0' : col_stop%16+'W');
                string sCol1 = cRoc+cCol1, sCol2 = "00"+cCol2, sRow1 = "00"+cRow1, sRow2 = "00"+cRow2;
                hexMask.insert(0," ").insert(0,sRow2).insert(0," ").insert(0,sCol2).insert(0," ").insert(0,sRow1).insert(0," ").insert(0,sCol1).insert(0,"d");
                for (uint16_t nRows(row_start); nRows <= row_stop; nRows++){
                    for (uint16_t nCols(col_start); nCols <= col_stop; nCols++) {
                        m_api->_dut->maskPixel(nCols, nRows, false, roc);
                        }}
                cout << "unmasked box from col "<< col_start <<" to " << col_stop;
                cout << " and row " << row_start << " to " << row_stop << " of ROC" << roc <<"\n";} //end elseif
            else if (maskPixels[i].id() == "cornTop") ; //catch identifier to avoid the error message

            else {
                cout << "Wrong identifier in maskFile!\n";
                EUDAQ_WARN("Wrong identifier in maskFile!");}//end else
        }//end first if
    }//end of loop

    /**complete hexMask*/
//    string hashBin;
//    string hashHex;
//    for (int i(0); i<80;i++){
//        for (int j(0); j<52;j++) {hashBin.push_back(maskArray[j][i]+'0');}
//    }
//    uint16_t it(3), hex(0);
//    uint32_t index(0);
//    while (index<hashBin.size()){
//        hex += pow(2,it)*(hashBin[index]-'0');
//        if (!it){
//            if (hex < 10 )  hashHex.push_back(hex+'0');
//            else            hashHex.push_back(hex+'W');
//            hex = 0; it = 4;
//        }
//        it--; index++;
//    }
    //cout << hashHex << "\n";
    cout << hexMask << "\n";

    /**print the name of the maskfile*/
    string filename = m_config.Get("maskFile", "");
    m_last_mask_filename = filename;
    stringstream maskname; maskname << "Using maskfile " << filename;
    EUDAQ_INFO(maskname.str());

    /**print number of masked pixels on each ROC*/
    for (uint16_t roc(0); roc<m_api->_dut->getNEnabledRocs();roc++){
        cout << "--> masked " << m_api->_dut->getNMaskedPixels(roc) << " Pixels in ROC " << roc << "!\n";
        stringstream ss; ss << "--> masked " << m_api->_dut->getNMaskedPixels(roc) << " Pixels in ROC "<< roc << " " << readHash(hexMask,char(roc+48));
        EUDAQ_INFO(hexMask);
        EUDAQ_INFO(ss.str()); //string("--> masked ")+m_api->_dut->getNMaskedPixels(roc)+string(" Pixels in ROC ")+roc
    }

    // Read DUT info, should print above filled information:
    m_api->_dut->info();

    std::cout << "Current DAC settings:" << std::endl;
    m_api->_dut->printDACs(0);

    if(!m_trimmingFromConf)
      SetStatus(eudaq::Status::LVL_WARN, "Couldn't read trim parameters from \"" + config.Name() + "\".");
    else
      SetStatus(eudaq::Status::LVL_OK, "Configured (" + config.Name() + ")");
    std::cout << "=============================\nCONFIGURED\n=============================" << std::endl;
  }
  catch (pxar::InvalidConfig &e){
    EUDAQ_ERROR(string("Invalid configuration settings: " + string(e.what())));
    SetStatus(eudaq::Status::LVL_ERROR, string("Invalid configuration settings: ") + e.what());
    delete m_api;
    m_api = NULL;
  }
  catch (pxar::pxarException &e){
    EUDAQ_ERROR(string("pxarCore Error: " + string(e.what())));
    SetStatus(eudaq::Status::LVL_ERROR, string("pxarCore Error: ") + e.what());
    delete m_api;
    m_api = NULL;
  }
  catch (...) {
    EUDAQ_ERROR(string("Unknown exception."));
    SetStatus(eudaq::Status::LVL_ERROR, "Unknown exception.");
    delete m_api;
    m_api = NULL;
  }
}

/**STARTRUN*/

void CMSPixelProducer::OnStartRun(unsigned runnumber) {
  m_run = runnumber;
  m_ev = 0;
  m_ev_filled = 0;
  m_ev_runningavg_filled = 0;

  try {
    EUDAQ_INFO("Switching Sensor Bias HV ON.");
    EUDAQ_INFO(m_last_mask_filename);
    m_api->HVon();

    std::cout << "Start Run: " << m_run << std::endl;

    eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE(m_event_type, m_run));
    // Set the TBM & ROC type for decoding:
    bore.SetTag("ROCTYPE", m_roctype);
    bore.SetTag("TBMTYPE", m_tbmtype);
    bore.SetTag("MASKFILE",m_last_mask_filename);

    // Set the number of planes (ROCs):
    bore.SetTag("PLANES", m_nplanes);

    // Store all DAC settings in one BORE tag:
    bore.SetTag("DACS", m_alldacs);

    // Set the PCB mount type for correct coordinate transformation:
    bore.SetTag("PCBTYPE", m_pcbtype);

    // Set the detector for correct plane assignment:
    bore.SetTag("DETECTOR", m_detector);

    // Store the pxarCore version this has been recorded with:
    bore.SetTag("PXARCORE", m_api->getVersion());
    // Store eudaq library version:
    bore.SetTag("EUDAQ", PACKAGE_VERSION);

    // Send the event out:
    SendEvent(bore);

    std::cout << "BORE with detector " << m_detector << " (event type " << m_event_type << ") and ROC type " << m_roctype << std::endl;

    // Start the Data Acquisition:
    m_api -> daqStart();




    // Send additional ROC Reset signal at run start:
    if(!m_api->daqSingleSignal("resetroc")) {
      throw InvalidConfig("Unable to send ROC reset signal!");
    }
    else {
      EUDAQ_INFO(string("ROC Reset signal issued."));
    }



    // If we run on Pattern Generator, activate the PG loop:
    if(m_trigger_is_pg) {
      m_api -> daqTriggerLoop(m_pattern_delay);
      triggering = true;
    }

    // Start the timer for period ROC reset:
    m_reset_timer = new eudaq::Timer;

    SetStatus(eudaq::Status::LVL_USER, "Running - HV ON!");
    m_running = true;
  }
  catch (...) {
    EUDAQ_ERROR(string("Unknown exception."));
    SetStatus(eudaq::Status::LVL_ERROR, "Unknown exception.");
    delete m_api;
    m_api = NULL;
  }
}

// This gets called whenever a run is stopped
void CMSPixelProducer::OnStopRun() {
  // Break the readout loop
  m_running = false;
  std::cout << "Stopping Run" << std::endl;

  try {
    // Acquire lock for pxarCore instance:
    std::lock_guard<std::mutex> lck(m_mutex);

    // If running with PG, halt the loop:
    if(m_trigger_is_pg) {
      m_api->daqTriggerLoopHalt();
      triggering = false;
    }

    // Wait before we stop the DAQ because TLU takes some time to pick up the OnRunStop signal
    // otherwise the last triggers get lost.
    eudaq::mSleep(m_tlu_waiting_time);

    // Stop the Data Acquisition:
    m_api->daqStop();

    EUDAQ_INFO("Switching Sensor Bias HV OFF.");
    m_api->HVoff();

    try {
      // Read the rest of events from DTB buffer:
      std::vector<pxar::rawEvent> daqEvents = m_api->daqGetRawEventBuffer();
      std::cout << "CMSPixel " << m_detector << " Post run read-out, sending " << daqEvents.size() << " evt." << std::endl;
      for(size_t i = 0; i < daqEvents.size(); i++) {
	eudaq::RawDataEvent ev(m_event_type, m_run, m_ev);
	ev.AddBlock(0, reinterpret_cast<const char*>(&daqEvents.at(i).data[0]), sizeof(daqEvents.at(i).data[0])*daqEvents.at(i).data.size());
	SendEvent(ev);
	if(daqEvents.at(i).data.size() > 1) { m_ev_filled++; }
	m_ev++;
      }
    }
    catch(pxar::DataNoEvent &) {
      // No event available in derandomize buffers (DTB RAM),
    }

    // Sending the final end-of-run event:
    SendEvent(eudaq::RawDataEvent::EORE(m_event_type, m_run, m_ev));
    std::cout << "CMSPixel " << m_detector << " Post run read-out finished." << std::endl;
    std::cout << "Stopped" << std::endl;

    // Output information for the logbook:
    std::cout << "RUN " << m_run << " CMSPixel " << m_detector << std::endl
	      << "\t Total triggers:   \t" << m_ev << std::endl
	      << "\t Total filled evt: \t" << m_ev_filled << std::endl;
    std::cout << "\t " << m_detector << " yield: \t"
	      << (m_ev > 0 ? std::to_string(100*m_ev_filled/m_ev) : "(inf)") << "%" << std::endl;
    EUDAQ_USER(string("Run " + std::to_string(m_run) + ", detector " + m_detector
		      + " yield: " + (m_ev > 0 ? std::to_string(100*m_ev_filled/m_ev) : "(inf)")
		      + "% (" + std::to_string(m_ev_filled) + "/" + std::to_string(m_ev) + ")"));

    SetStatus(eudaq::Status::LVL_OK, "Stopped");
  } catch (const std::exception & e) {
    printf("While Stopping: Caught exception: %s\n", e.what());
    SetStatus(eudaq::Status::LVL_ERROR, "Stop Error");
  } catch (...) {
    printf("While Stopping: Unknown exception\n");
    SetStatus(eudaq::Status::LVL_ERROR, "Stop Error");
  }
}

void CMSPixelProducer::OnTerminate() {

  std::cout << "CMSPixelProducer terminating..." << std::endl;
  // Stop the readout loop, force routine to return:
  m_terminated = true;

  // If we already have a pxarCore instance, shut it down cleanly:
  if(m_api != NULL) {
    delete m_api;
    m_api = NULL;
  }

  std::cout << "CMSPixelProducer " << m_producerName << " terminated." << std::endl;
}

void CMSPixelProducer::ReadoutLoop() {
  // Loop until Run Control tells us to terminate
  while (!m_terminated) {
    // No run is m_running, cycle and wait:
    if(!m_running) {
      // Move this thread to the end of the scheduler queue:
      sched_yield();
      continue;
    }
    else {
      // Acquire lock for pxarCore object access:
      std::lock_guard<std::mutex> lck(m_mutex);


      // Send periodic ROC Reset
      if(m_roc_resetperiod > 0 && m_reset_timer->mSeconds() > m_roc_resetperiod) {
	if(!m_api->daqSingleSignal("resetroc")) { EUDAQ_ERROR(string("Unable to send ROC reset signal!\n")); }
	m_reset_timer->Restart();
      }

      // Trying to get the next event, daqGetRawEvent throws exception if none is available:
      try {
	pxar::rawEvent daqEvent = m_api->daqGetRawEvent();

	eudaq::RawDataEvent ev(m_event_type, m_run, m_ev);
	ev.AddBlock(0, reinterpret_cast<const char*>(&daqEvent.data[0]), sizeof(daqEvent.data[0])*daqEvent.data.size());

	SendEvent(ev);
	m_ev++;
	// Events with pixel data have more than 4 words for TBM header/trailer and 1 for each ROC header:
	if(daqEvent.data.size() > (4 + m_nplanes)) { m_ev_filled++; m_ev_runningavg_filled++; }

	// Print every 1k evt:
	if(m_ev%1000 == 0) {
	  uint8_t filllevel = 0;
	  m_api->daqStatus(filllevel);
	  std::cout << "CMSPixel " << m_detector
		    << " EVT " << m_ev << " / " << m_ev_filled << " w/ px" << std::endl;
	  std::cout << "\t Total average:  \t" << (m_ev > 0 ? std::to_string(100*m_ev_filled/m_ev) : "(inf)") << "%" << std::endl;
	  std::cout << "\t 1k Trg average: \t" << (100*m_ev_runningavg_filled/1000) << "%" << std::endl;
	  std::cout << "\t RAM fill level: \t" << static_cast<int>(filllevel) << "%" << std::endl;
	  m_ev_runningavg_filled = 0;
	}
      }
      catch(pxar::DataNoEvent &) {
	// No event available in derandomize buffers (DTB RAM), return to scheduler:
	sched_yield();
      }
    }
  }
}


/** Read hash */
string CMSPixelProducer::readHash(string hexMask, char i2c){

    stringstream ss;

    for (uint16_t i(0); i<hexMask.size(); i++){
        if (hexMask[i]==i2c && i % 5 == 1){
            if (hexMask[i-1] == 'a'){
                ss << "pix" << int(hexMask[i+1])*16+int(hexMask[i+2]) << "/" << int(hexMask[i+6])*16+int(hexMask[i+7]);
            }
            else if (hexMask[i-1] == 'b'){
                uint16_t start = (hexMask[i+1]-'0')*16 + (hexMask[i+2] < '9'? hexMask[i+2]-'0' : hexMask[i+2]-'W');
                uint16_t stop  = (hexMask[i+6]-'0')*16 + (hexMask[i+7] < '9'? hexMask[i+7]-'0' : hexMask[i+7]-'W');
                start == stop ? ss << "col " << start << ", " : ss << "col " << start << "-" << stop << ", ";
            }
            else if (hexMask[i-1] == 'c'){
                uint16_t start = (hexMask[i+1]-'0')*16 + (hexMask[i+2] < '9'? hexMask[i+2]-'0' : hexMask[i+2]-'W');
                uint16_t stop  = (hexMask[i+6]-'0')*16 + (hexMask[i+7] < '9'? hexMask[i+7]-'0' : hexMask[i+7]-'W');
                start == stop ? ss << "row " << start << ", " : ss << "row " << start << "-" << stop << ", ";}
            else if (hexMask[i-1] == 'd'){
                uint16_t start1 = (hexMask[i+1]-'0')*16 + (hexMask[i+2] < '9'? hexMask[i+2]-'0' : hexMask[i+2]-'W');
                uint16_t start2  = (hexMask[i+6]-'0')*16 + (hexMask[i+7] < '9'? hexMask[i+7]-'0' : hexMask[i+7]-'W');
                uint16_t stop1 = (hexMask[i+11]-'0')*16 + (hexMask[i+12] < '9'? hexMask[i+12]-'0' : hexMask[i+12]-'W');
                uint16_t stop2  = (hexMask[i+16]-'0')*16 + (hexMask[i+17] < '9'? hexMask[i+17]-'0' : hexMask[i+17]-'W');
                ss << "unmasked box: from " << start1 << "/" << start2 << " to " << stop1 << "/" << stop2;
            }}}
    return ss.str();
}

// The main function that will create a Producer instance and run it
int main(int /*argc*/, const char ** argv) {
  // You can use the OptionParser to get command-line arguments
  // then they will automatically be described in the help (-h) option
  eudaq::OptionParser op("CMS Pixel Producer", "0.0",
			 "Run options");
  eudaq::Option<std::string> rctrl(op, "r", "runcontrol",
				   "tcp://localhost:44000", "address", "The address of the RunControl.");
  eudaq::Option<std::string> level(op, "l", "log-level", "NONE", "level",
				   "The minimum level for displaying log messages locally");
  eudaq::Option<std::string> name (op, "n", "name", "CMSPixel", "string",
				   "The name of this Producer");
  eudaq::Option<std::string> verbosity(op, "v", "verbosity mode", "INFO", "string");
  try {
    // This will look through the command-line arguments and set the options
    op.Parse(argv);
    // Set the Log level for displaying messages based on command-line
    EUDAQ_LOG_LEVEL(level.Value());
    // Create a producer
    CMSPixelProducer producer(name.Value(), rctrl.Value(), verbosity.Value());
    // And set it running...
    producer.ReadoutLoop();
    // When the readout loop terminates, it is time to go
    std::cout << "Quitting" << std::endl;
  } catch (...) {
    // This does some basic error handling of common exceptions
    return op.HandleMainException();
  }
  return 0;
}

