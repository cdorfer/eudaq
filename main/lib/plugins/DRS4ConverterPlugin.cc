#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/StandardEvent.hh"
#include "eudaq/Utils.hh"
#include <string.h>
#include <stdio.h>

// All LCIO-specific parts are put in conditional compilation blocks
// so that the other parts may still be used if LCIO is not available.
#if USE_LCIO
#  include "IMPL/LCEventImpl.h"
#  include "IMPL/TrackerRawDataImpl.h"
#  include "IMPL/LCCollectionVec.h"
#  include "lcio.h"
#endif

namespace eudaq {

// The event type for which this converter plugin will be registered
// Modify this to match your actual event type (from the Producer)
static const char* EVENT_TYPE = "DRS4";

// Declare a new class that inherits from DataConverterPlugin
class DRS4ConverterPlugin : public DataConverterPlugin {
private:
	int m_serial_no;
	int m_firmware;
	int m_board_type;
	int m_n_channels;
	unsigned char m_activated_channels;
	std::map<int, std::string> m_channel_names;
	std::string m_dut_name;
public:
	// This is called once at the beginning of each run.
	// You may extract information from the BORE and/or configuration
	// and store it in member variables to use during the decoding later.
	virtual void Initialize(const Event & bore,
			const Configuration & cnf) {
		std::cout<<"Read DRS4 Bore Event"<<std::endl;
#ifndef WIN32  //some linux Stuff //$$change
		(void)cnf; // just to suppress a warning about unused parameter cnf
#endif
		m_serial_no = bore.GetTag("DRS4_serial_no", (int)-1);
		m_firmware = bore.GetTag("DRS4_FW",(int)-1);
		m_board_type = bore.GetTag("DRS4_Board_type", (int)-1);
		m_n_channels = bore.GetTag("DRS4_n_channels", (int)-1);
		m_activated_channels = bore.GetTag("activated_channels",(unsigned char)0);
		m_dut_name = bore.GetTag("device_name","DRS4");
		std::cout<<"  serial_no:  "<<m_serial_no<<std::endl;
		std::cout<<"  firmware:   "<<m_firmware<<std::endl;
		std::cout<<"  board_type: "<<m_board_type<<std::endl;
		std::cout<<"  n_channels: "<<m_n_channels<<":"<<std::endl;
		for (int ch = 0; ch< m_n_channels;ch++){
					std::string tag = "CH_"+std::to_string(ch+1);
					m_channel_names[ch] = bore.GetTag(tag,tag);
					std::cout<<"    "<<tag<<": "<<m_channel_names[ch]<<std::endl;
				}
		//todo set range
	}

	// Here, the data from the RawDataEvent is extracted into a StandardEvent.
	// The return value indicates whether the conversion was successful.
	// Again, this is just an example, adapted it for the actual data layout.
	virtual bool GetStandardSubEvent(StandardEvent & sev,
			const Event & ev) const {
//		std::cout<<"\nDRS4::GetStandardSubEvent"<<std::endl;
		const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(ev);
		int nblocks = in_raw.NumBlocks();
//		std::cout<<"Number of Blocks: "<<nblocks<<std::endl;
		// If the event type is used for different sensors
		// they can be differentiated here
		// Create a StandardPlane representing one sensor plane
		int id = 0;
		// Get Trigger cell
		RawDataEvent::data_t data = in_raw.GetBlock(id++);
		int trigger_cell = static_cast<int>(data[0]);
		// Get Timestamp
		data = in_raw.GetBlock(id++);
		uint64_t timestamp = *((uint64_t*) &data[0]);
//		sev.SetTimestamp(timestamp);

		float min_waves[m_n_channels];
		float max_waves[m_n_channels];
		//Get Raw data
//		std::cout<<"Read Event: "<<nblocks<<" "<<m_n_channels<<" @ "<<timestamp<<std::endl;
		for (id = id; id < nblocks;){
			// Get Header
//			std::cout<<"Get Header:"<<std::endl;
			data = in_raw.GetBlock(id++);
			char buffer [5];
			std::memcpy(&buffer,&(data[0]), 4);
			buffer[4]='\0';
			int ch = atoi(&buffer[1])-1;
//			std::cout<<"buffer: "<<buffer<<"\t=> ch:"<<ch<<std::endl;
//			std::cout<<"Get Event Data of CH_"<<ch+1<<" - "<<m_channel_names.at(ch)<<std::endl;

			//Get Waveform
			data = in_raw.GetBlock(id++);
			int wave_size = data.size();
			int n_samples =  wave_size/sizeof(unsigned short);
//			std::cout<<"CH: "<<ch<<" with "
//					<<data.size()<<" -> "<<n_samples<<"  .";//<<std::endl;
//			std::cout<<"Trigger cell "<<trigger_cell<<", ";

			unsigned short *raw_wave_array = (unsigned short*) &data[0];
			float wave_array[n_samples];
			//Conversion of raw data to voltage data
			for (int i = 0; i < n_samples; i++)
				wave_array[i] = (raw_wave_array[i] / 65536. + range/1000.0 - 0.5)*1000.;
//			min_waves[ch] = *std::min_element(wave_array,wave_array+n_samples);
//			max_waves[ch] = *std::max_element(wave_array,wave_array+n_samples);
//			std::cout<<"From: "<< min_waves[ch] << " mV to " << max_waves[ch] << " mV"<<std::endl;
			//conversion of time:
//			 for (j=0,time[chn_index][i]=0 ; j<i ; j++)
//			               time[chn_index][i] += bin_width[chn_index][(j+eh.trigger_cell) % 1024];
			//add Waveform to standard event
			StandardWaveform wf(ch,EVENT_TYPE,m_dut_name+(std::string)"_"+m_channel_names.at(ch));
			wf.SetChannelName(m_channel_names.at(ch));
			wf.SetChannelNumber(ch);
			wf.SetNSamples(n_samples);
			wf.SetWaveform((float*) wave_array);
			wf.SetTimeStamp(timestamp);
			sev.AddWaveform(wf);
//			std::cout<<"CH"<<ch<<": "<<wf<<std::endl;
			// Indicate that data was successfully converted
		}
//		std::cout<<sev<<std::endl;
//		std::cout<<"Standard Event with "<<sev.NumWaveforms()<<" Waveforms."<<std::endl;
		return true;
	}

#if USE_LCIO
	// This is where the conversion to LCIO is done
	virtual lcio::LCEvent * GetLCIOEvent(const Event * /*ev*/) const {
		return 0;
	}
#endif

private:
	unsigned short range;
	// The constructor can be private, only one static instance is created
	// The DataConverterPlugin constructor must be passed the event type
	// in order to register this converter for the corresponding conversions
	// Member variables should also be initialized to default values here.
	DRS4ConverterPlugin(): DataConverterPlugin(EVENT_TYPE),
			m_serial_no(-1),
			m_firmware(-1),
			m_board_type(-1),
			m_n_channels(0),
			m_dut_name(""),
			range(0)
	{
//		std::cout<<"DRS4ConverterPlugin Constructor"<<std::endl;
	}


	// The single instance of this converter plugin
	static DRS4ConverterPlugin m_instance;

}; // class DRS4ConverterPlugin

// Instantiate the converter plugin instance
DRS4ConverterPlugin DRS4ConverterPlugin::m_instance;

} // namespace eudaq
