/*
 * WaveformCollection.hh
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#ifndef WaveformCollection_HH_
#define WaveformCollection_HH_
//ROOT Includes
#include <RQ_OBJECT.h>
#include <TH2I.h>
#include <TFile.h>
//#include <TGraph.h>

//STL includes
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <iostream>

//Project Includes
#include "SimpleStandardEvent.hh"
#include "WaveformHistos.hh"
#include "BaseCollection.hh"
#include "WaveformOptions.hh"

class WaveformCollection : public BaseCollection
{
	RQ_OBJECT("WaveformCollection")
protected:
	bool isOneWaveformRegistered;
	std::map<SimpleStandardWaveform,WaveformHistos*> _map;
	bool isWaveformRegistered(SimpleStandardWaveform p);
	void fillHistograms(const SimpleStandardWaveform &simpWaveform);
	WaveformOptions *_WaveformOptions;
public:
	WaveformCollection() : BaseCollection(), _WaveformOptions(0)
	{
		std::cout << " Initialising Waveform Collection"<<std::endl;
		isOneWaveformRegistered = false;
		CollectionType = HITMAP_COLLECTION_TYPE;
	}
	virtual ~WaveformCollection() {};
	void registerWaveform(const SimpleStandardWaveform &p);
	void registerGlobalWaveforms(const SimpleStandardWaveform &p,int wf_type=0);
	void registerDataWaveforms(const SimpleStandardWaveform &p,std::string prefix,std::string desc);
	void registerBadFFTWaveforms(const SimpleStandardWaveform &p,std::string desc);
	void registerBadFFTWaveforms(const SimpleStandardWaveform &p);
	void registerSignalWaveforms(const SimpleStandardWaveform &p);
	void registerPulserWaveforms(const SimpleStandardWaveform &p);
	void bookHistograms(const SimpleStandardEvent &simpev);
	void setRootMonitor(RootMonitor *mon)  {_mon = mon; }
	void Fill(const SimpleStandardEvent &simpev);
	WaveformHistos * getWaveformHistos(std::string sensor, int id);
	void SetWaveformOptions(WaveformOptions *options){_WaveformOptions = options;}
	void Reset();
	virtual void Write(TFile *file);
	virtual void Calculate(const unsigned int currentEventNumber);
};

#ifdef __CINT__
#pragma link C++ class WaveformCollection-;
#endif

#endif /* WaveformCollection_HH_ */
