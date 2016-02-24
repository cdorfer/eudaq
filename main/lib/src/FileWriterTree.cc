#ifdef ROOT_FOUND
 
#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/PluginManager.hh"
#include "eudaq/Logger.hh"
#include "eudaq/FileSerializer.hh"

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

  class FileWriterTree : public FileWriter {
  public:
    FileWriterTree(const std::string &);
    virtual void StartRun(unsigned);
    virtual void WriteEvent(const DetectorEvent &);
    virtual uint64_t FileBytes() const;
    virtual ~FileWriterTree();
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
    std::vector<int> * f_plane;
    std::vector<int> * f_col;
    std::vector<int> * f_row;
    std::vector<int> * f_adc;
    std::vector<int> * f_charge;  
    
  };

  namespace {
    static RegisterFileWriter<FileWriterTree> reg("tree");
  }

  FileWriterTree::FileWriterTree(const std::string & /*param*/)
    : m_tfile(0), m_ttree(0),m_noe(0),chan(4),n_pixels(90*90+60*60)
  {

    f_plane  = new std::vector<int>;
    f_col    = new std::vector<int>;	  
    f_row    = new std::vector<int>;	  
    f_adc    = new std::vector<int>;	  
    f_charge = new std::vector<int>;  

  }

  void FileWriterTree::StartRun(unsigned runnumber) {
    EUDAQ_INFO("Converting the inputfile into a TTree(felix format) " );
    std::string foutput(FileNamer(m_filepattern).Set('X', ".root").Set('R', runnumber));
    EUDAQ_INFO("Preparing the outputfile: " + foutput);
    m_tfile = new TFile(foutput.c_str(), "RECREATE");
    m_ttree = new TTree("tree", "a simple Tree with simple variables");

    // Set Branch Addresses
    m_ttree->Branch("event_number",&f_event_number, "event_number/I");
    m_ttree->Branch("time",&f_time, "time/F");

    m_ttree->Branch("plane", &f_plane);
    m_ttree->Branch("col", &f_col);
    m_ttree->Branch("row", &f_row);
    m_ttree->Branch("adc", &f_adc);
    m_ttree->Branch("charge", &f_charge);

  }

  void FileWriterTree::WriteEvent(const DetectorEvent & ev) {

    if (ev.IsBORE()) {
      eudaq::PluginManager::Initialize(ev);
      //firstEvent =true;
      return;
    } else if (ev.IsEORE()) {
      return;
    }

    StandardEvent sev = eudaq::PluginManager::ConvertToStandard(ev);

    f_event_number = sev.GetEventNumber();
    f_time = sev.GetTimestamp()/384066.;

    f_plane->clear();
    f_col->clear();
    f_row->clear();
    f_adc->clear();
    f_charge->clear();

    for (size_t iplane = 0; iplane < sev.NumPlanes(); ++iplane) {
      
      const eudaq::StandardPlane & plane = sev.GetPlane(iplane);
      std::vector<double> cds = plane.GetPixels<double>();
      
      for (size_t ipix = 0; ipix < cds.size(); ++ipix) {
	
	f_plane->push_back(iplane);
	f_col->push_back(plane.GetX(ipix));
	f_row->push_back(plane.GetY(ipix));
	f_adc->push_back((int)plane.GetPixel(ipix));
	f_charge->push_back(42);	
      }
    }

    m_ttree->Fill();
    
  }


  FileWriterTree::~FileWriterTree() {
    std::cout<<"Tree has " << m_ttree->GetEntries() << " entries" << std::endl;
    m_ttree->Write();
    if(m_tfile->IsOpen()) m_tfile->Close();
  }

  uint64_t FileWriterTree::FileBytes() const { return 0; }

}
#endif // ROOT_FOUND
