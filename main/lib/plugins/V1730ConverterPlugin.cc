#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/StandardEvent.hh"
#include "eudaq/Utils.hh"
#include <string.h>
#include <stdio.h>
#include <algorithm>


namespace eudaq{

//event type for which the converter will be used
static const char* EVENT_TYPE = "V1730";


class V1730ConverterPlugin:public DataConverterPlugin {


public:
  virtual void Initialize(const Event & bore, const Configuration & cnf) {
	std::cout<<"V1730 Initialize"<<std::endl;
	//m_serialno = bore.GetTag("V1730_serial_no", (int)-1);
	//m_firmware = bore.GetTag("V1730_firmware_v", (int)-1);
	//m_timestamp = bore.GetTag("V1730_timestamp", (int)-1);
	m_range = 0.5; //TODO

}

  virtual bool GetStandardSubEvent(StandardEvent & sev, const Event & ev) const {
//	std::cout << "\nV1730: GetStandardSubEvent" << std::endl;
	
	const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(ev);
	int nblocks = in_raw.NumBlocks();
//	std::cout << "Number of Blocks: " << nblocks << std::endl;

	//get data:
	int id = 0;
	RawDataEvent::data_t data = in_raw.GetBlock(id);
	bool valid_bit = static_cast<bool>(data[0]); //valid bit
//	std::cout << "Valid bit: " << valid_bit << std::endl;
	id++;

	data = in_raw.GetBlock(id);
	uint64_t timestamp = *((uint64_t*) &data[0]); //timestamp
//	std::cout << "Timestamp: " << timestamp << std::endl;
	sev.SetTimestamp(timestamp);
	id++;

	data = in_raw.GetBlock(id);
	uint32_t event_counter = static_cast<uint32_t>(data[0]); //event counter
//	std::cout << "Event counter: " << event_counter << std::endl;
	id++;


	//get waveforms:
	int data_size, n_samples;
	int n_channels = nblocks - 3;

	for(int ch = 0; ch < n_channels; ch++){
	  data = in_raw.GetBlock(id);
	  data_size = data.size(); 
	  n_samples =  data_size/sizeof(uint16_t); //change from /sizeof(uint16_t)

	  std::cout << "Waveform data size: " << data_size << std::endl;
	  std::cout << "Number of samples per waveform: " << n_samples << std::endl;

	  uint16_t wave_array[n_samples];
	  //uint16_t *raw_wave_array = (uint16_t*) &data[0];
	  uint16_t *raw_wave_array = (uint16_t*)(&data[0]);

	  for (int i = 0; i < n_samples; i++){
//	  	wave_array[i] = raw_wave_array[i];}

	  	wave_array[i] = (uint16_t)(m_range*1000*raw_wave_array[i]/16384); //m_range=0.5V, 1000 fuer mV.
	  	if(wave_array[i] == 0){
	  		std::cout << "data is zero at channel " << ch << "at sample " << i << std::endl;
	  	}
	  }

	  StandardWaveform wf(ch, EVENT_TYPE, "CH" + std::to_string(ch));
	  wf.SetChannelName("CH" + std::to_string(ch));
	  wf.SetChannelNumber(ch);
	  wf.SetNSamples(n_samples);
	  wf.SetWaveform((uint16_t*) wave_array);
	  sev.AddWaveform(wf);
	  id++;
	}//end ch loop

	return true;
}







private:
  V1730ConverterPlugin() : DataConverterPlugin(EVENT_TYPE), m_serialno(-1), m_firmware(-1), m_range(0){}

  uint64_t m_timestamp;
  uint32_t m_serialno;
  float m_firmware, m_range;
  static V1730ConverterPlugin m_instance;

}; // class V1730ConverterPlugin

V1730ConverterPlugin V1730ConverterPlugin::m_instance;
} // namespace eudaq
