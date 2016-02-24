/*
 * SimpleStandardWaveform.hh
 *
 *  Created on: Jun 9, 2011
 *      Author: stanitz
 */

#ifndef SimpleStandardWaveform_HH_
#define SimpleStandardWaveform_HH_

#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <algorithm>
#include <numeric>      // std::accumulate

#include <stdio.h>
#include <string.h>
#include <numeric>

#include <TROOT.h>
#include "include/OnlineMonConfiguration.hh"
#include <TVirtualFFT.h>
#include <TMath.h>

//!Simple Standard Waveform Class
/*!

 */
class SimpleStandardWaveform {
protected:
	std::string _name;
	std::string _channelname;
	int _id;
	ULong64_t _timestamp;
	int _tlu_event;
	int _channelnumber;
public:
	SimpleStandardWaveform(const std::string & name, const int id, const unsigned int nsamples, OnlineMonConfiguration* mymon);
	SimpleStandardWaveform(const std::string & name, const int id, const unsigned int nsamples);
	virtual ~SimpleStandardWaveform() {};
	std::string getName() const {return _name;}
	std::string getChannelName() const{return _channelname;}
	void setChannelName(std::string channelname){_channelname = channelname;}
	void setChannelNumber(int channelno){_channelnumber = channelno;};
	void addSuffix( const std::string & suf ) { _name = _name + suf; }
	int getID() const {return _id;}
	void addData(float *data);
	void setMonitorConfiguration(OnlineMonConfiguration *mymon)
	{
		mon=mymon;
	}
	void Calculate();
	float getMax() const{return !calculated?getMaximum(0,1e9):_max;};
	float getMin()const{return !calculated?getMinimum(0,1e9):_min;};
	float getIntegral() const{return !calculated?getIntegral(0,_nsamples):_integral;}
	float getIntegral(float min, float max) const;
    float maxSpreadInRegion(float min, float max) const;
	float getAbsMaximum(float min, float max) const;
	float getMaximum(float min, float max) const;
	float getMinimum(float min, float max) const;
	void setNSamples(unsigned int n_samples){_nsamples = n_samples;}
	unsigned int getNSamples() const{return _nsamples;}
	float* getData() const{return _data;}
	int getSign() const {return _sign;}
	void setSign(int sign){ _sign = sign>0?1:-1;}
	void setTimestamp(ULong64_t timestamp){_timestamp = timestamp;};
	ULong64_t getTimestamp() const {return _timestamp;}
	void setEvent(int event){_tlu_event = event;}
	int getEvent() const{return _tlu_event;}
	bool isPulserEvent() const{ return _PulserEvent;}
	void setPulserEvent() {setPulserEvent(true);}
	void setPulserEvent(bool pulserEvent) {_PulserEvent = pulserEvent;}//std::cout<<"setPulser:"<<_PulserEvent<<std::endl;}
    void setMeanFFT(float mean_fft) {_mean_fft = mean_fft;}
    void setMaxFFT(float max_fft)   {_max_fft = max_fft;}
    void setMinFFT(float min_fft)   {_min_fft = min_fft;}
    float getMeanFFT() const{return _mean_fft;}
    float getMaxFFT () const{return _max_fft;}
    float getMinFFT () const{return _min_fft;}
    void performFFT(TVirtualFFT *);
private:
	int _sign;
	bool calculated;
	bool _PulserEvent;
	float *_data;
	float _mean_fft;
	float _max_fft;
	float _min_fft;
	float _max;
	float _min;
	float _integral;
	unsigned int _nsamples;
	OnlineMonConfiguration* mon;
};

#endif //ifndef SimpleStandardWaveform_HH_
