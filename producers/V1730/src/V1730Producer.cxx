// system includes:
#include <iostream>
#include <ostream>
#include <vector>
#include <mutex>
#include <cmath>
#include <string>
#include <unistd.h>
#include <algorithm>


// EUDAQ includes:
#include "eudaq/Producer.hh"
#include "eudaq/Logger.hh"
#include "eudaq/RawDataEvent.hh"
#include "eudaq/Timer.hh"
#include "eudaq/Utils.hh"
#include "eudaq/OptionParser.hh"
#include "eudaq/Configuration.hh"

#include "vme/Concurrent/VP717.h"
#include "V1730Producer.hh"
#include "vme/CAEN/V1730_ADC/caen_v1730.h"


static const std::string EVENT_TYPE = "V1730";


V1730Producer::V1730Producer(const std::string & name, const std::string & runcontrol, const std::string & verbosity)  
  : eudaq::Producer(name, runcontrol),
  m_producerName(name),
  m_event_type(EVENT_TYPE),
  m_ev(0), 
  m_run(0), 
  m_running(false){

  if (V1730_handle)
    delete V1730_handle;
  
  try{
  //initialize a pointer to the caen_v1730 class:
  vp717_interface = new VP717();
  interface = vp717_interface;
  vme_addr_t V1730_handleBaseAddress=0x32100000;
  V1730_handle = new caen_v1730(interface,V1730_handleBaseAddress);
  
  //get serial number and firmware revision and initialize class variables:
  m_serialno = V1730_handle->getSerialNumber();
  caen_v1730::firmware_revision fw = V1730_handle->caen_v1730::getROC_firmware();
  m_firmware = fw.firmware_major + 0.1*fw.firmware_minor;

  std::cout << "V1730: VME connection to CAEN " << V1730_handle->getBoardModel() << " established." << std::endl;
  std::cout << "Firmware version: " << m_firmware << std::endl << std::endl;

  m_terminated = false;
  }
  catch (...){
    EUDAQ_ERROR(std::string("Error in the V1730Producer class constructor."));
    SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1730Producer class constructor.");}
}





void V1730Producer::OnStartRun(unsigned runnumber){
  m_run = runnumber;
  m_ev = 0;
  try{
    //set time stamp:
    this->SetTimeStamp();

    //create event:
    std::cout<<"V1730: Create " << m_event_type << " EVENT for run " << m_run <<  " @time: " << m_timestamp << "." << std::endl;
    eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE(m_event_type, m_run));
    bore.SetTag("V1730_timestamp", m_timestamp);
    bore.SetTag("V1730_serial_no", m_serialno);
    bore.SetTag("V1730_firmware_v", m_firmware);
    
    for (unsigned ch = 0; ch <  V1730_handle->GROUPS; ch++){
        std::string key = "V1730_CHANNEL_DAC_"+std::to_string(ch);
	std::cout << key << " - " << m_channel_dac.at(ch);
	bore.SetTag(key, m_channel_dac.at(ch));
	
        key = "V1730_CHANNEL_RANGE_"+std::to_string(ch);
	std::cout << key << " - " << m_dynamic_range.at(ch);
	bore.SetTag(key, m_dynamic_range.at(ch));

        key = "V1730_CHANNEL_GAIN_"+std::to_string(ch);
	std::cout << key << " - " << m_channel_gain.at(ch);
	bore.SetTag(key, m_channel_gain.at(ch));
    }
    //set number of channels to be implemented
    //set tags for the channel numbers

    //buffer in event??

    //set digitizer online (allow it to accept triggers)
    V1730_handle->Start();

    V1730_handle->printAcquisitionStatus(V1730_handle->getAcquisitionStatus());

    SendEvent(bore);

    SetStatus(eudaq::Status::LVL_OK, "Running");
    m_running = true;
  }
  catch (...){
  EUDAQ_ERROR(std::string("Error in the V1730 OnStartRun procedure."));
  SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1730 OnStartRun procedure.");}
}


void V1730Producer::OnStopRun(){
  // Break the readout loop
  m_running = false;
  V1730_handle->Stop();
  std::cout << "V1730 run stopped." << std::endl;
}



void V1730Producer::OnTerminate(){
  m_running = false;
  m_terminated = true;
  V1730_handle->Stop();
  V1730_handle->SoftwareReset(); //resets registers
  V1730_handle->SoftwareClear(); //clears buffers
  std::cout << "V1730 run terminated, registers reset and buffers cleared." << std::endl;
  delete vp717_interface, interface, V1730_handle;
}



void V1730Producer::ReadoutLoop() {
  while(!m_terminated){
    if(!m_running){
      sched_yield();} //if not running deprioritize thread


  while(m_running){
    try{
      //std::cout << "Readout loop running.." << std::endl;
      //V1730_handle->SoftwareTrigger();
      //sleep(1);

      if(V1730_handle->isEventReady()){
        //set time stamp:
        this->SetTimeStamp();
        
        bool event_valid = true;
        v1730_event event; //CAEN event:


        V1730_handle->ReadEvent_D32(event);
        if(!event.isValid()){
          event_valid = false;
        }

        uint32_t event_counter = event.EventCounter();
        //std::cout << "#######" << std::endl;
        //std::cout << "Event valid:           " << event.isValid()           << std::endl;
        //std::cout << "Event size(32b words): " << event.EventSize()          << std::endl;
        //std::cout << "Channel mask:          " << event.ChannelMask()        << std::endl;
        std::cout << "\rEvent Counter:         " << event_counter << "/" << m_ev  << std::flush;
        //std::cout << "Samples per channel:   " << event.SamplesPerChannel()  << std::endl;
        //std::cout << "Channels:              " << event.Channels()           << std::endl;
        //std::cout << "TimeStamp:             " << event.TriggerTimeTag()     << std:: endl;
        //std::cout << std::endl << std::endl; 

        eudaq::RawDataEvent ev(m_event_type, m_run, event_counter); //generate a raw event
        ev.SetTimeStampToNow(); // Let's think weather this information can help us...
        ev.SetTag("TriggerTimeTag", event.TriggerTimeTag());

        unsigned int block_no = 0;
        ev.AddBlock(block_no, reinterpret_cast<const char*>(&event_valid), sizeof(bool)); //valid bit
        block_no++;

        ev.AddBlock(block_no, reinterpret_cast<const char*>(&m_timestamp), sizeof(m_timestamp)); //timestamp
        block_no++;


        ev.AddBlock(block_no, reinterpret_cast<const char*>(&m_ev), sizeof(uint32_t)); //event counter
        block_no++;


        //get information regarding the number and size of the events
        uint32_t n_channels = event.Channels();
        //std::cout << "Number of Channels: " << n_channels << std::endl;
        size_t n_samples = event.SamplesPerChannel();

        //read out all channels
        for(uint32_t ch = 0; ch < n_channels; ch++){
          uint16_t *payload = new uint16_t[n_samples];
          event.getChannelData(ch, payload, n_samples);
          //std::cout << "Size of readout - n_samples: " << n_samples << ", payload [byte]: " << n_samples*sizeof(uint16_t) << std::endl;
          ev.AddBlock(block_no, reinterpret_cast<const char*>(payload), n_samples*sizeof(uint16_t));
          block_no++;
          delete payload;
        }
        
        SendEvent(ev);
	      m_ev++;

        //std::cout << "Event counter (loop): " << m_ev << std::endl;      
      }//end if
      
    }  
    catch (...){
    EUDAQ_ERROR(std::string("Readout error................"));
    SetStatus(eudaq::Status::LVL_ERROR, "Readout error................");}

    }//running
  
  }//terminated
}




void V1730Producer::OnConfigure(const eudaq::Configuration& conf) {
  std::cout << "###Configure V1730 board:" << std::endl;  
  m_config = conf;

  m_trigger_source = m_config.Get("trigger_source", 1); //default 1 for external trigger
  m_active_channels = m_config.Get("active_channels", 1); //default 1 only for ch1
  m_trigger_threshold = m_config.Get("trigger_threshold", 1); //default 1
  m_post_trigger_samples = m_config.Get("post_trigger_samples", 0); //default0

  try{
    if(V1730_handle->isRunning()){
      V1730_handle->Stop();}
    
    //set all configuration registers to their default values
    V1730_handle->SoftwareReset();
    sleep(1);

    //set trigger source
    caen_v1730::trigger_mask t_mask = V1730_handle->getTriggerSourceMask();
    t_mask.raw = m_trigger_source;
    V1730_handle->setTriggerSourceMask(t_mask);
    V1730_handle->printTriggerMask(V1730_handle->getTriggerSourceMask());

    //enable channels
    caen_v1730::channel_enable_mask c_enable_mask = caen_v1730::channel_enable_mask(0);
    c_enable_mask.raw = m_active_channels;
    V1730_handle->setChannelEnableMask(c_enable_mask);
    V1730_handle->printChannelEnableMask(V1730_handle->getChannelEnableMask());

    //send busy signal via the TRG OUT Limo connnector
    caen_v1730::front_panel_io_control_reg io_control = V1730_handle->getFrontPanelIOControl();
    uint32_t io_mask0 = io_control.raw;
    std::cout << "IO mask before: " << io_mask0 << std::endl;
    io_control.probe_select = 3;
    io_control.trg_out_mode_sel = 1;
    V1730_handle->setFrontPanelIOControl(io_control);

    //short check
    io_control = V1730_handle->getFrontPanelIOControl();
    uint32_t io_mask1 = io_control.raw;
    std::cout << "IO mask after: " << io_mask1 << std::endl << std::endl;


    //set buffer organization (1024 memory blocks for 1024 events)
    V1730_handle->setBufferOrganisation(0x0A);
    std::cout << "Buffer Organisation: 0x" << std::hex << uint32_t(V1730_handle->getBufferOrganisation()) << std::dec << std::endl;

    //set custom event lenght
    int length = 990;
	  V1730_handle->setBufferCustomSize((length+10)/10);
    std::cout << "Buffer setBufferCustomSize: " << V1730_handle->getBufferCustomSize() << std::endl;

    //set post trigger samples:
    V1730_handle->setPostTriggerSamples(m_post_trigger_samples);
	  std::cout << "Post trigger samples set to: " << m_post_trigger_samples << std::endl;

    //set almost full level to 800 events (out of 1024)
    V1730_handle->setAlmostFullLevel(800);

    //count all triggers (not just accepted ones)
    caen_v1730::acq_control acq_ctrl = V1730_handle->getACQControl();
    acq_ctrl.count_mode = 1;  //count all triggers
    acq_ctrl.board_full_mode = 1; //full when (N-1) events are reached
    V1730_handle->setACQControl(acq_ctrl);

    //allow event overlap
    caen_v1730::channel_configuration ch_config = V1730_handle->getGroupConfig();
    ch_config.trigger_overlapping = 1;
    V1730_handle->setGroupConfig(ch_config);


    //set channel voltage range
    for(int ch = 0; ch < V1730_handle->GROUPS; ch++){
      V1730_handle->setChannel_Gain(ch, caen_v1730::GAIN_4); //set input range to +/-0.5V
      V1730_handle->setChannel_Thres(ch, m_trigger_threshold); //set trigger threshold
      std::cout << "Calibrating channel " << ch << "..";
      V1730_handle->doChannel_Calibration(ch);
      caen_v1730::channel_status status = V1730_handle->getChannel_Status(ch);
      if(status.calibration_status){
        std::cout << " [OK] - "; 
        //std::cout << " [OK] - Temp: " << V1730_handle->getChannel_Temperature(ch) << std::endl;
        float dac = V1730_handle->getChannel_DAC(ch);
        float range =  V1730_handle->getChannel_DynamicRange(ch);
        float gain =  V1730_handle->getChannel_Gain(ch);
        m_dynamic_range[ch] = range;
        m_channel_dac[ch] = dac;
        m_channel_gain[ch] = gain;
        std::cout << "Range: " << range << ", dac: " << dac << ", gain: " << gain << std::endl;
      }
      else{
        std::cout << "[FAILED]"<<std::endl;
      }
      
    }




  std::cout << "V1730: Configured! Ready to take data." << std::endl;
  
  SetStatus(eudaq::Status::LVL_OK, "Configured V1730");

  }catch ( ... ){
  EUDAQ_ERROR(std::string("Error in the V1730 configuration procedure."));
   SetStatus(eudaq::Status::LVL_ERROR, "Error in the V1730 configuration procedure.");}
}




int main(int /*argc*/, const char ** argv){
  // You can use the OptionParser to get command-line arguments
  // then they will automatically be described in the help (-h) option
  eudaq::OptionParser op("V1730 Producer", "0.0", "Run options");
  eudaq::Option<std::string> rctrl(op, "r", "runcontrol",
    "tcp://localhost:44000", "address", "The address of the RunControl.");
  eudaq::Option<std::string> level(op, "l", "log-level", "NONE", "level",
    "The minimum level for displaying log messages locally");
  eudaq::Option<std::string> name (op, "n", "name", "V1730", "string",
    "The name of this Producer");
  eudaq::Option<std::string> verbosity(op, "v", "verbosity mode", "INFO", "string");

  try{
    // This will look through the command-line arguments and set the options
    op.Parse(argv);
    // Set the Log level for displaying messages based on command-line
    EUDAQ_LOG_LEVEL(level.Value());
    // Create a producer
    V1730Producer *producer = new V1730Producer(name.Value(), rctrl.Value(), verbosity.Value());
    // And set it running...
    producer->ReadoutLoop();
    // When the readout loop terminates, it is time to go
    std::cout << "V1730: Quitting" << std::endl;

  }
  catch(...){
    // This does some basic error handling of common exceptions
    return op.HandleMainException();}

  return 0;
}


void V1730Producer::SetTimeStamp(){
  std::chrono::high_resolution_clock::time_point epoch;
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed = now - epoch;
  m_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()/100u;
}




