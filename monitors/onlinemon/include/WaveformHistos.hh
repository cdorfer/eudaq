/*
 * WaveformHistos.hh
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#ifndef WaveformHISTOS_HH_
#define WaveformHISTOS_HH_
//ROOT
//#include <TGraph.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TFile.h>
#include <TString.h>
#include <THStack.h>
#include <TGraph.h>
//std
#include <map>
#include <algorithm>    // std::min_element, std::max_element
#include <cstdlib>
//eudaq
#include "SimpleStandardEvent.hh"
#include "TGraphSet.hh"
#include "WaveformOptions.hh"


class RootMonitor;

class WaveformHistos {
  protected:
    std::string _sensor;
    int _id;
    bool _wait;
    std::vector<TH1F*> _Waveforms;
    std::vector<TH1F*> _GoodWaveforms;
    std::vector<TH1F*> _BadFFTWaveforms;
    THStack* h_wf_stack;
    THStack* h_goodwf_stack;
    THStack* h_badfftwf_stack;
    int _n_wfs;
    unsigned int _n_samples;
    std::map<std::string, TH1*> profiles;
    std::map<std::string, TH1*> time_profiles;
    std::map<std::string, TH1*> histos;
    std::map<std::string, std::pair<float,float> > rangesX;
    std::map<std::string, std::pair<float,float> > rangesY;
    ULong64_t time_start;
    // categories:
    // 0: good Event
    // 1: flat line Event
    // 2: badFFt Event
    // 3: pulser Event
    enum EventCategroy{
        UNKNOWN_EVENT = -1,
        GOOD_EVENT=0,
        FLAT_EVENT=1,
        BAD_FFT_MAX_EVENT=2,
        BAD_FFT_MEAN_EVENT=3,
        BAD_FFT_BOTH_EVENT=4,
        PULSER_EVENT=5,
    };
  public:
    WaveformHistos(SimpleStandardWaveform p, RootMonitor * mon);
    std::string getName() const {return (std::string)TString::Format("%s_%d",_sensor.c_str(),_id);};
    virtual ~WaveformHistos(){}
    void Fill(const SimpleStandardWaveform & wf);
    void FillEvent(const SimpleStandardWaveform & wf, bool isPulserEvent);
    unsigned int getNSamples() const {return _n_samples;}
    void Reset();
    void SetOptions(WaveformOptions* options){_config = options;std::cout<<"Setting Options for "<<getName()<<std::endl;};

    void Calculate(const int currentEventNum);
    void Write();
    unsigned int GetNWaveforms() const {return _n_wfs;};
    TH1F * getWaveformGraph(int i) { return _Waveforms[i%_n_wfs]; }
    TH1F * getBadFFTWaveformGraph(int i) { return _BadFFTWaveforms[i%_n_wfs]; }
    THStack* getWaveformStack(){return h_wf_stack;}
    THStack* getBadFFTWaveformStack(){return h_badfftwf_stack;}
    THStack* getGoodWaveformStack(){return h_goodwf_stack;}
    void setRootMonitor(RootMonitor *mon)  {_mon = mon; };
    // signal histos   
    TH1F* getFullAverageVoltageHisto() const { return (TH1F*)histos.at("FullAverage");};
    TH1F* getSignalHisto() const{ return (TH1F*)histos.at("Signal");};
    TH1F* getSignalMinusPedestalHisto() const{ return (TH1F*)histos.at("SignalMinusPedestal");};
    TH1F* getPedestalHisto() const { return (TH1F*)histos.at("Pedestal");};
    // TH1F* getDeltaIntegralVoltageHisto() const { return (TH1F*)histos.at("DeltaIntegral");};
    TProfile* getProfileFullAverageVoltage() const { return (TProfile*)profiles.at("FullAverage");};
    //TProfile* getProfileDeltaIntegral() const { return (TProfile*)profiles.at("DeltaIntegral");};
    TProfile* getProfileSignal() const { return (TProfile*)profiles.at("Signal");};
    TProfile* getProfileSignalMinusPedestal() const { return (TProfile*)profiles.at("SignalMinusPedestal");};
    TProfile* getProfilePedestal() const { return (TProfile*)profiles.at("Pedestal");};
    TProfile* getProfile(std::string key) const;
    // pulser histos   
    TH1F* getPulserFullAverageVoltageHisto() const { return (TH1F*)histos.at("Pulser_FullAverage");};
    TH1F* getPulserSignalHisto() const{ return (TH1F*)histos.at("Pulser_Signal");};
    TH1F* getPulserSignalMinusPedestalHisto() const{ return (TH1F*)histos.at("Pulser_SignalMinusPedestal");};
    TH1F* getPulserPedestalHisto() const { return (TH1F*)histos.at("Pulser_Pedestal");};
    // TH1F* getPulserDeltaIntegralVoltageHisto() const { return (TH1F*)histos.at("Pulser_DeltaIntegral");};
    TProfile* getPulserProfileFullAverageVoltage() const { return (TProfile*)profiles.at("Pulser_FullAverage");};
    //TProfile* getPulserProfileDeltaIntegral() const { return (TProfile*)profiles.at("Pulser_DeltaIntegral");};
    TProfile* getPulserProfileSignal() const { return (TProfile*)profiles.at("Pulser_Signal");};
    TProfile* getPulserProfilePedestal() const { return (TProfile*)profiles.at("Pulser_Pedestal");};
    TProfile* getPulserProfileSignalMinusPedestal() const { return (TProfile*)profiles.at("Pulser_SignalMinusPedestal");};
    TProfile* getPulserProfile(std::string key) const;
    TH2F* getCategoryVsEventHisto() const { return (TH2F*)histos.at("CategoryVsEvent");}

    TH1* getHisto(std::string key) const;
    TProfile* getTimeProfile(std::string key) const;
    void SetMaxRangeX(std::string,float minx, float maxx);
    void SetMaxRangeY(std::string,float min, float max);
    void SetPedestalIntegralRange(float min, float max);
    void SetSignalIntegralRange(float min, float max);

  private:
    std::pair<float,float> pedestal_integral_range;
    std::pair<float,float> signal_integral_range;
    std::pair<float,float> pulser_integral_range;
    unsigned int n_fills;
    unsigned int n_fills_good;
    unsigned int n_fills_bad;
    void InitHistos();
    void InitIntegralHistos();
    void InitFFTHistos();
    void InitBadFFTHistos();
    void InitSpreadHistos();
    void InitProfiles();
    void InitTimeProfiles();
    void InitPulserProfiles();
    void InitSignalProfiles();
    void InitPedestalProfiles();
    void InitWaveformStacks();
    void Reinitialize_Waveforms();
    void Reinitialize_BadFFTWaveforms();
    void Reinitialize_GoodWaveforms();
    void UpdateRanges();
    void UpdateRange(TH1* histo);
    int SetHistoAxisLabelx(TH1* histo,std::string xlabel);
    int SetHistoAxisLabely(TH1* histo,std::string ylabel);
    int SetHistoAxisLabels(TH1* histo,std::string xlabel, std::string ylabel);
    RootMonitor * _mon;
    WaveformOptions* _config;
    bool do_fitting;
    float min_wf;
    float max_wf;
};

#ifdef __CINT__
#pragma link C++ class WaveformHistos-;
#endif


#endif /* WaveformHISTOS_HH_ */

