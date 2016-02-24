#ifdef ROOT_FOUND
 
#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/PluginManager.hh"
#include "eudaq/Logger.hh"
#include "eudaq/FileSerializer.hh"


#include "include/SimpleStandardEvent.hh"

//# include<inttypes.h>
# include "TFile.h"
# include "TTree.h"
# include "TRandom.h"
# include "TString.h"

using namespace std;

template <typename T>
inline T unpack_fh (vector <unsigned char >::iterator &src, T& data){ //unpack from host-byte-order
  data=0;
  for(unsigned int i=0;i<sizeof(T);i++){
    data+=((uint64_t)*src<<(8*i));
    src++;
  }
  return data;
}

template <typename T>
inline T unpack_fn(vector<unsigned char>::iterator &src, T& data){            //unpack from network-byte-order
  data=0;
  for(unsigned int i=0;i<sizeof(T);i++){
    data+=((uint64_t)*src<<(8*(sizeof(T)-1-i)));
    src++;
  }
  return data;
}

template <typename T>
inline T unpack_b(vector<unsigned char>::iterator &src, T& data, unsigned int nb){            //unpack number of bytes n-b-o only
  data=0;
  for(unsigned int i=0;i<nb;i++){
    data+=(uint64_t(*src)<<(8*(nb-1-i)));
    src++;
  }
  return data;
}

typedef pair<vector<unsigned char>::iterator,unsigned int> datablock_t;


namespace eudaq {

  class FileWriterTreeCAEN : public FileWriter {
  public:
    FileWriterTreeCAEN(const std::string &);
    virtual void StartRun(unsigned);
    virtual void WriteEvent(const DetectorEvent &);
    virtual uint64_t FileBytes() const;
    float Calculate(std::vector<float> * data, int min, int max);
    virtual ~FileWriterTreeCAEN();
  private:
    TFile * m_tfile; // book the pointer to a file (to store the otuput)
    TTree * m_ttree; // book the tree (to store the needed event info)
    // Book variables for the Event_to_TTree conversion
    unsigned m_noe;
    short chan;
    int n_pixels;

    // Scalar Branches     
    int f_event_number;
    float f_time;

    // Vector Branches     
    float f_ped;
    float f_sig;
    float f_adc;
    float f_charge;  
    
  };

  namespace {
    static RegisterFileWriter<FileWriterTreeCAEN> reg("caentree");
  }

  FileWriterTreeCAEN::FileWriterTreeCAEN(const std::string & /*param*/)
    : m_tfile(0), m_ttree(0),m_noe(0),chan(4),n_pixels(90*90+60*60)
  {

    f_ped    = 0.;	  
    f_sig    = 0.;	  
    f_adc    = 0.;	  
    f_charge = 0.;  

  }

  float FileWriterTreeCAEN::Calculate(std::vector<float> * data, int min, int max) {
    float integral = 0;
    int i;
    for (i = min; i <= int(max+1) && i < data->size() ;i++){
        integral += data->at(i);
    }
    return integral/(float)(i-(int)min);
  }

  void FileWriterTreeCAEN::StartRun(unsigned runnumber) {
    EUDAQ_INFO("Converting the inputfile into a CAEN TTree(marc format) " );
    std::string foutput(FileNamer(m_filepattern).Set('X', "_caen.root").Set('R', runnumber));
    EUDAQ_INFO("Preparing the outputfile: " + foutput);
    m_tfile = new TFile(foutput.c_str(), "RECREATE");
    m_ttree = new TTree("tree", "a simple Tree with simple variables");

    // Set Branch Addresses
    m_ttree->Branch("event_number",&f_event_number, "event_number/I");

    m_ttree->Branch("time",&f_time, "time/F");

    m_ttree->Branch("ped", &f_ped, "pedestal/F");
    m_ttree->Branch("sig", &f_sig, "signal/F");
    m_ttree->Branch("adc", &f_adc, "signal/F");
    m_ttree->Branch("charge", &f_charge, "charge/F");

  }

  void FileWriterTreeCAEN::WriteEvent(const DetectorEvent & ev) {

    if (ev.IsBORE()) {
      eudaq::PluginManager::Initialize(ev);
      //firstEvent =true;
      return;
    } else if (ev.IsEORE()) {
      return;
    }

    StandardEvent sev = eudaq::PluginManager::ConvertToStandard(ev);

    unsigned int nwf = (unsigned int) sev.NumWaveforms();
    // std::cout << "number of wf in my event" << nwf << std::endl;

    // SimpleStandardEvent simpEv;
    // simpEv.setEvent_number(ev.GetEventNumber());
    // simpEv.setEvent_timestamp(ev.GetTimestamp());

    //initialize with default configuration
//    mon_configdata.SetDefaults();
    // print the configuration
//    mon_configdata.PrintConfiguration();


    std::vector<float> * data;


    // assuming exactly one waveform for (unsigned int i = 0; i < nwf;i++){
        const eudaq::StandardWaveform & waveform = sev.GetWaveform(0);
            std::string sensorname;
            sensorname = waveform.GetType();
            // SimpleStandardWaveform simpWaveform(sensorname,waveform.ID());
            // simpWaveform.setNSamples(waveform.GetNSamples());
            // std::cout << "number of samples in my wf" << waveform.GetNSamples() << std::endl;
            data = waveform.GetData();
            // std::cout << "this is data " << data << endl;
            // std::cout << "this is data->at(3) " << data->at(3) << endl;
            
            float sig = Calculate(data, 1075, 1150);
            float ped = Calculate(data,    1,  900);
            f_sig= sig;
            f_ped= ped;

    data->clear();

            // simpWaveform.addData(&(*waveform.GetData())[0]);
            // // simpWaveform.Calculate();
            // simpWaveform.setTimestamp(ev.GetTimestamp());
            // simpWaveform.setEvent(ev.GetEventNumber());
            // simpWaveform.setChannelName(waveform.GetChannelName());
            // simpWaveform.setChannelNumber(waveform.GetChannelNumber());
            // // simpEv.addWaveform(simpWaveform);




    f_event_number = sev.GetEventNumber();
    f_time = sev.GetTimestamp()/384066.;

    // f_ped->clear();
    // f_sig->clear();
    // f_adc->clear();
    // f_charge->clear();

    // for (size_t iplane = 0; iplane < sev.NumPlanes(); ++iplane) {
    //   
    //   const eudaq::StandardPlane & plane = sev.GetPlane(iplane);
    //   std::vector<double> cds = plane.GetPixels<double>();
    //   
    //   for (size_t ipix = 0; ipix < cds.size(); ++ipix) {
	// 
	// f_plane->push_back(iplane);
	// f_col->push_back(plane.GetX(ipix));
	// f_row->push_back(plane.GetY(ipix));
	// f_adc->push_back((int)plane.GetPixel(ipix));
	// f_charge->push_back(42);	
    //   }
    // }

    m_ttree->Fill();
    
  }


  FileWriterTreeCAEN::~FileWriterTreeCAEN() {
    std::cout<<"Tree has " << m_ttree->GetEntries() << " entries" << std::endl;
    m_ttree->Write();
    if(m_tfile->IsOpen()) m_tfile->Close();
  }

  uint64_t FileWriterTreeCAEN::FileBytes() const { return 0; }

}
#endif // ROOT_FOUND
