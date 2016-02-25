//written by dorfer@phys.ethz.ch
#ifndef TUPRODUCER_HH
#define TUPRODUCER_HH

//Readout_Data struct defined here:
#include "triger_logic_tpc_stream.h"

class Producer;
class Configuration;
class trigger_controll;


class TUProducer:public eudaq::Producer{
public:
	TUProducer(const std::string &name, const std::string &runcontrol, const std::string &verbosity);
	virtual void MainLoop();
	virtual void OnConfigure(const eudaq::Configuration &conf);
	virtual void OnStartRun(unsigned param);
	virtual void OnStopRun();
	virtual void OnTerminate();
	virtual void OnReset();
	virtual void OnStatus();
	float SlidingWindow(float);


private:
	std::string producer_name, event_type;
	unsigned int m_run, m_ev, m_ev_prev, prev_coincidence_count; //run & event number
	bool done, TUStarted, TUJustStopped;
	trigger_controll *tc; //class for TU control from trigger_controll.h
	Triger_Logic_tpc_Stream *stream; //class for handling communication from triger_logic_tpc_stream.h
	int trg_mask;
	float cal_beam_current;
	std::deque<float> avg;

	//data read back from TU
	unsigned int trigger_counts[10];
	unsigned int prev_trigger_counts[10];
	unsigned int input_frequencies[10];
	unsigned int trigger_counts_multiplicity[10];
	unsigned int coincidence_count_no_sin;
	unsigned int coincidence_count;
	unsigned int beam_current[2]; //first entry = old, second entry = new
	unsigned int prescaler_count;
	unsigned int prescaler_count_xor_pulser_count;
	unsigned int pulser_delay_and_xor_pulser_count;
	unsigned int handshake_count;
	unsigned long time_stamps[2]; //first entry = old, second entry = new timestamp

};

int main(int /*argc*/, const char ** argv);

#endif