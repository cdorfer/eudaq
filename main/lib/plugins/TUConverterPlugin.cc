#include "eudaq/DataConverterPlugin.hh"
#include "eudaq/TUEvent.hh"
#include <string>

namespace eudaq{

    static const char* EVENT_TYPE = "TU";

    class TUConverterPlugin:public DataConverterPlugin{
        public:
            TUConverterPlugin() : DataConverterPlugin(Event::str2id("_TU")){} //changed from "_TU"
            virtual unsigned GetTriggerID(const eudaq::Event & ev) const{return ev.GetEventNumber();}
            virtual bool GetStandardSubEvent(eudaq::StandardEvent &sev, const eudaq::Event &ev) const{
                const RawDataEvent & in_raw = dynamic_cast<const RawDataEvent &>(ev);
                sev.SetTimestamp(ev.GetTimestamp());
                //sev.SetTag("TLU_trigger",ev.GetTag("trigger"));

                /*
                int nblocks = in_raw.NumBlocks();
                int id = 0;
                RawDataEvent::data_t data = in_raw.GetBlock(id);
                valid = static_cast<bool>(data[0]);

                if(valid){
                    id++;

                    data = in_raw.GetBlock(id);
                    coincidence_count_no_sin = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    coincidence_count = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    cal_beam_current = static_cast<float>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    prescaler_count = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    prescaler_count_xor_pulser_count = static_cast<unsigned int>(data[0]);
                    id++;

                    data = in_raw.GetBlock(id);
                    handshake_count = static_cast<unsigned int>(data[0]);
                    id++;
                }
                */

                return true;
            }


        private:
            bool valid;
            unsigned int coincidence_count_no_sin;
            unsigned int coincidence_count;
            float cal_beam_current;
            unsigned int prescaler_count;
            unsigned int prescaler_count_xor_pulser_count;
            unsigned int handshake_count;
            static TUConverterPlugin const m_instance;
        };

    TUConverterPlugin const TUConverterPlugin::m_instance;

}//namespace eudaq



