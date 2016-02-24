/*
 * WaveformHistos.cc
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#include "WaveformHistos.hh"
#include "OnlineMon.hh"
#include <TStyle.h>
#include <TROOT.h>

#include <math.h>

WaveformHistos::WaveformHistos(SimpleStandardWaveform p, RootMonitor * mon):
_n_wfs(10),_sensor(p.getName()), _id(p.getID()),n_fills(0),n_fills_bad(0),n_fills_good(0),_n_samples(1024),time_start(0)
{
    char out[1024], out2[1024];
    _mon=mon;
    min_wf = 1e9;
    max_wf = -1e9;
    do_fitting = false;

    // signal_integral_range = make_pair(500.,800.);
    //signal_integral_range   = make_pair(  200, 350.);
    //signal_integral_range = make_pair(140, 220.); //changed to this by cdorfer on oct 31
    signal_integral_range = make_pair(120, 180.); /**changed to this by mreichmann on nov 13*/
    //pedestal_integral_range = make_pair(   10, 160.);// should be the same length as signal
//    pulser_integral_range = make_pair(760,860);
    pulser_integral_range = make_pair(880,920); /**changed to this by mreichmann on nov 13*/
    pedestal_integral_range = make_pair(20, 100.);//changed to this by cdorfer on oct 31
    //	std::cout << "WaveformHistos::Sensorname: " << _sensor << " "<< _id<< std::endl;
    this->InitHistos();
}


void WaveformHistos::InitHistos() {
//    std::cout<<"WaveformHistos::InitHistos"<<std::endl;
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;
    TString hName, hTitle;
    hName = TString::Format("h_SignalEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: no of signal events ; is signal event; number of entries",_sensor.c_str(),_id);
    histos["SignalEvents"]= new TH1F(hName,hTitle,2,-.5,1.5);

    hName = TString::Format("h_BadFFTEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: no of badFFT events ; is badFFT event; number of entries",_sensor.c_str(),_id);
    histos["BadFFTEvents"]= new TH1F(hName,hTitle,2,-.5,1.5);

    hName = TString::Format("h_PulserEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: no of pulser events ; is pulser event; number of entries",_sensor.c_str(),_id);
    histos["PulserEvents"]= new TH1F(hName,hTitle,2,-.5,1.5);

    hName = TString::Format("h_nFlatLineEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: no of flat line events ; number of entries",_sensor.c_str(),_id);
    histos["nFlatLineEvents"]= new TH1F(hName,hTitle,2,-.5,1.5);

    hName = TString::Format("h_nBadFFTEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: no of bad events acc to FFT cuts ; number of entries",_sensor.c_str(),_id);
    histos["nBadFFTEvents"]= new TH1F(hName,hTitle,2,-.5,1.5);

    hName = TString::Format("hCategoryVsEventNo_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d:Categroy Vs Event No.; Event No.; Category",_sensor.c_str(),_id);
    histos["CategoryVsEvent"] = new TH2F(hName,hTitle,1000,0,1000,7,-1,6);
    histos["CategoryVsEvent"]->SetBit(TH1::kCanRebin);
    /*
        UNKNOWN_EVENT = -1,
        GOOD_EVENT=0,
        FLAT_EVENT=1,
        BAD_FFT_MAX_EVENT=2,
        BAD_FFT_MEAN_EVENT=3,
        PULSER_EVENT=4,
     */
    histos["CategoryVsEvent"]->GetYaxis()->SetBinLabel(1,"UNKNOWN");
    histos["CategoryVsEvent"]->GetYaxis()->SetBinLabel(2,"Good");
    histos["CategoryVsEvent"]->GetYaxis()->SetBinLabel(3,"Flat");
    histos["CategoryVsEvent"]->GetYaxis()->SetBinLabel(4,"FFT_{max}");
    histos["CategoryVsEvent"]->GetYaxis()->SetBinLabel(5,"FFT_{mean}");
    histos["CategoryVsEvent"]->GetYaxis()->SetBinLabel(6,"FFT_{both}");
    histos["CategoryVsEvent"]->GetYaxis()->SetBinLabel(7,"Pulser");
    histos["CategoryVsEvent"]->SetStats(false);

    hName = TString::Format("hCategories_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Category;Category; number of entries",_sensor.c_str(),_id);
    histos["Category"] = new TH1F(hName,hTitle,7,-1,6);
    histos["Category"]->GetXaxis()->SetBinLabel(1,"UNKNOWN");
    histos["Category"]->GetXaxis()->SetBinLabel(2,"Good");
    histos["Category"]->GetXaxis()->SetBinLabel(3,"Flat");
    histos["Category"]->GetXaxis()->SetBinLabel(4,"FFT_{max}");
    histos["Category"]->GetXaxis()->SetBinLabel(5,"FFT_{mean}");
    histos["Category"]->GetXaxis()->SetBinLabel(6,"FFT_{both}");
    histos["Category"]->GetXaxis()->SetBinLabel(7,"Pulser");

    InitIntegralHistos();
    InitFFTHistos();
    InitBadFFTHistos();
    InitSpreadHistos();
    InitProfiles();
    InitWaveformStacks();
}

void WaveformHistos::InitIntegralHistos(){
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;
    //=====================================================================
    //======== SIGNAL HISTOGRAMS ==========================================
    //=====================================================================
    TString hName, hTitle;
    hName = TString::Format("h_SignalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Signal Integral Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,signal_integral_range.first,signal_integral_range.second);
    histos["SignalIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_PedestalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pedestal Integral Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,pedestal_integral_range.first,pedestal_integral_range.second);
    histos["PedestalIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_PulserIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser Integral Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,pulser_integral_range.first,pulser_integral_range.second);
    histos["PulserIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_Pulser_SignalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Signal Integral Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,signal_integral_range.first,signal_integral_range.second);
    histos["Pulser_SignalIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_Pulser_PedestalIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pedestal Integral Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,pedestal_integral_range.first,pedestal_integral_range.second);
    histos["Pulser_PedestalIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_Pulser_PulserIntegral_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser Integral Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,pulser_integral_range.first,pulser_integral_range.second);
    histos["Pulser_PulserIntegral"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

}
void WaveformHistos::InitFFTHistos(){
    //=====================================================================
    //======== FFT HISTOGRAMS =============================================
    //=====================================================================
    TString hName, hTitle;
    hName = TString::Format("h_MeanFFT_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Mean FFT Value ; number of entries",
            _sensor.c_str(),_id);
    histos["MeanFFT"] = new TH1F(hName,hTitle,2000,0.,2000.);
    //SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_InvMaxFFT_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Inv Max FFT Value ; number of entries",
            _sensor.c_str(),_id);
    histos["InvMaxFFT"] = new TH1F(hName,hTitle,10000,0.000001,0.005);
    //SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_Pulser_MeanFFT_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser Mean FFT Value ; number of entries",
            _sensor.c_str(),_id);
    histos["Pulser_MeanFFT"] = new TH1F(hName,hTitle,2000,0.,2000.);
    //SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_Pulser_InvMaxFFT_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser Inv Max FFT Value ; number of entries",
            _sensor.c_str(),_id);
    histos["Pulser_InvMaxFFT"] = new TH1F(hName,hTitle,10000,0.000001,0.005);
    //SetMaxRangeX((string)hName,-50,500);

}
void WaveformHistos::InitBadFFTHistos(){
    //=====================================================================
    //======== BAD HISTOGRAMS =============================================
    //=====================================================================
    TString hName, hTitle;
    hName = TString::Format("h_BadFFT_MeanFFT_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Mean FFT Value (bad events); number of entries",
            _sensor.c_str(),_id);
    histos["BadFFT_MeanFFT"] = new TH1F(hName,hTitle,2000,0.,2000.);
    //SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_BadFFT_InvMaxFFT_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Inv Max FFT Value (bad events); number of entries",
            _sensor.c_str(),_id);
    histos["BadFFT_InvMaxFFT"] = new TH1F(hName,hTitle,10000,0.000001,0.005);
    //SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_BadFFT_Signal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Signal (bad events); number of entries",
            _sensor.c_str(),_id);
    histos["BadFFT_Signal"] = new TH1F(hName,hTitle,1000,0.,500);
    //SetMaxRangeX((string)hName,-50,500);

}
void WaveformHistos::InitSpreadHistos(){
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;
    //=====================================================================
    //======== SIGNAL HISTOGRAMS ==========================================
    //=====================================================================
    TString hName, hTitle;

//    PulserEvents
    hName = TString::Format("h_FullAverage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: FullAverage; full average/mV; number of entries",_sensor.c_str(),_id);
    histos["FullAverage"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,50);

    hName = TString::Format("h_Pulser_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser range (%5.1f - %5.1f); pulser /mV ; number of entries",
            _sensor.c_str(),_id,pulser_integral_range.first,pulser_integral_range.second);
    histos["Pulser"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,50);

    hName = TString::Format("h_Signal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Signal Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,signal_integral_range.first,signal_integral_range.second);
    histos["Signal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_Pedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: PedestalRange (%5.1f - %5.1f); pedestal /mV; number of entries",
            _sensor.c_str(),_id,pedestal_integral_range.first,pedestal_integral_range.second);
    histos["Pedestal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    // SetMaxRangeX((string)hName,-30, 30);

    hName = TString::Format("h_SignalMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: SignalMinusPedestal; (signalSpread-pedestalSpread)/mV; number of entries",_sensor.c_str(),_id);
    histos["SignalMinusPedestal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);

    hName = TString::Format("h_PulserMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: PulserMinusPedestal; (pulserSpread-pedestalSpread)/mV; number of entries",_sensor.c_str(),_id);
    histos["PulserMinusPedestal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);

    //=====================================================================
    //======== PULSER HISTOGRAMS ==========================================
    //=====================================================================
    hName = TString::Format("h_Pulser_FullAverage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser FullAverage; full average/mV; number of entries",_sensor.c_str(),_id);
    histos["Pulser_FullAverage"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,50);

    hName = TString::Format("h_Pulser_Pulser_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser Pulser; pulser /mV (%5.1f - %5.1f); number of entries",
            _sensor.c_str(),_id,pulser_integral_range.first,pulser_integral_range.second);
    histos["Pulser_Pulser"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
//    SetMaxRangeX((string)hName,-50,50);

    hName = TString::Format("h_Pulser_Signal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser Signal Range (%5.1f - %5.1f); signal /mV; number of entries",
            _sensor.c_str(),_id,signal_integral_range.first,signal_integral_range.second);
    histos["Pulser_Signal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
    SetMaxRangeX((string)hName,-50,500);

    hName = TString::Format("h_Pulser_Pedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser Pedestal Range (%5.1f - %5.1f); pedestal /mV; number of entries",
            _sensor.c_str(),_id,pedestal_integral_range.first,pedestal_integral_range.second);
    histos["Pulser_Pedestal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);

    hName = TString::Format("h_Pulser_SignalMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser SignalMinusPedestal; (signalSpread-pedestalSpread)/mV; number of entries",_sensor.c_str(),_id);
    histos["Pulser_SignalMinusPedestal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);

    hName = TString::Format("h_Pulser_PulserMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Pulser PulserMinusPedestal; (pulserSpread-pedestalSpread)/mV; number of entries",_sensor.c_str(),_id);

    histos["Pulser_PulserMinusPedestal"] = new TH1F(hName,hTitle,nbins,minVolt,maxVolt);
}


void WaveformHistos::InitTimeProfiles() {
    TString hName = TString::Format("h_TimeProfileSignalMinusPedestal_%s_%d",_sensor.c_str(),_id);
    TString hTitle = TString::Format("%s %d: Time Profile Signal-Pedestal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            signal_integral_range.first,
            signal_integral_range.second);
    time_profiles["SignalMinusPedestal"] = new TProfile(hName,hTitle,1,0,10);
    time_profiles["SignalMinusPedestal"]->SetStats(false);

    hName = TString::Format("h_TimeProfilePulserSignalMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Time Profile Pulser Signal-Pedestal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            signal_integral_range.first,
            signal_integral_range.second);
    time_profiles["Pulser_SignalMinusPedestal"] = new TProfile(hName,hTitle,1,0,10);
    time_profiles["Pulser_SignalMinusPedestal"]->SetStats(false);

    std::map<std::string, TH1*>::iterator it;
    for (it = time_profiles.begin();it!=time_profiles.end();it++){
//        it->second->GetXaxis()->SetTimeDisplay(1);
//        it->second->GetXaxis()->SetTimeFormat("%M:%S");
    }
}

void WaveformHistos::InitProfiles(){
    InitTimeProfiles();
//    std::cout<<"WaveformHistos::InitProfiles"<<std::endl;
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;
    //=====================================================================
    //======== SIGNAL PROFILES   ==========================================
    //=====================================================================
    TString hName;
    TString hTitle;

    hName = TString::Format("h_ProfileSignalEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile no of SIgnalEvents; event number / 5000events; rel. no of SignalEvents",_sensor.c_str(),_id);
    profiles["SignalEvents"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["SignalEvents"]->SetStats(false);
    profiles["SignalEvents"]->GetYaxis()->SetRangeUser(0., 1.);

    hName = TString::Format("h_ProfileBadFFTEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile no of BadFFTEvents; event number / 5000events; rel. no of BadFFTEvents",_sensor.c_str(),_id);
    profiles["BadFFTEvents"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["BadFFTEvents"]->SetStats(false);
    profiles["BadFFTEvents"]->GetYaxis()->SetRangeUser(0., 1.);

    hName = TString::Format("h_ProfilePulserEvents_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile no of PulserEvents; event number / 5000events; rel. no of PulserEvents",_sensor.c_str(),_id);
    profiles["PulserEvents"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["PulserEvents"]->SetStats(false);
    profiles["PulserEvents"]->GetYaxis()->SetRangeUser(0., 1.);

    hName = TString::Format("h_ProfileFullAverage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile FullAverage; event number / 5000events; signal/mV",_sensor.c_str(),_id);
    profiles["FullAverage"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["FullAverage"]->SetStats(false);

    hName = TString::Format("h_ProfileSignal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Signal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            signal_integral_range.first,
            signal_integral_range.second);
    profiles["Signal"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Signal"]->SetStats(false);

    hName = TString::Format("h_ProfileSignalMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Signal-Pedestal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            signal_integral_range.first,
            signal_integral_range.second);
    profiles["SignalMinusPedestal"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["SignalMinusPedestal"]->SetStats(false);

    hName = TString::Format("h_ProfilePedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Pedestal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            pedestal_integral_range.first,
            pedestal_integral_range.second);
    profiles["Pedestal"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Pedestal"]->SetStats(false);

    InitPulserProfiles();
    // hName = TString::Format("h_ProfileDeltaIntegral_%s_%d",_sensor.c_str(),_id);
    // hTitle = TString::Format("%s %d: Profile Signal-PedestalIntegral; event number / 5000events; signal/mV",_sensor.c_str(),_id);
    // profiles["DeltaIntegral"] = new TProfile(hName,hTitle,1,0,1000);
    // profiles["DeltaIntegral"]->SetStats(false);
}

//=====================================================================
//======== PULSER PROFILES   ==========================================
//=====================================================================
void WaveformHistos::InitPulserProfiles(){
    float minVolt = -500; //-1000;
    float maxVolt = 500; //+1000;
    int nbins =  1000; //2000;
    TString hName = TString::Format("h_Pulser_ProfileDelta_%s_%d",_sensor.c_str(),_id);
    TString hTitle = TString::Format("%s %d: Profile Delta; event number / 5000events; signal/mV",_sensor.c_str(),_id);

    hName = TString::Format("h_Pulser_ProfileFullAverage_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile FullAverage; event number / 5000events; signal/mV",_sensor.c_str(),_id);
    profiles["Pulser_FullAverage"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Pulser_FullAverage"]->SetStats(false);

    hName = TString::Format("h_Pulser_ProfileSignal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Signal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            signal_integral_range.first,
            signal_integral_range.second);
    profiles["Pulser_Signal"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Pulser_Signal"]->SetStats(false);

    hName = TString::Format("h_Pulser_ProfileSignalMinusPedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Signal-Pedestal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            signal_integral_range.first,
            signal_integral_range.second);
    profiles["Pulser_SignalMinusPedestal"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Pulser_SignalMinusPedestal"]->SetStats(false);

    hName = TString::Format("h_Pulser_ProfilePulser_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Pulser (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            pulser_integral_range.first,
            pulser_integral_range.second);
    profiles["Pulser_Pulser"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Pulser_Pulser"]->SetStats(false);

    hName = TString::Format("h_ProfilePulser_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Pulser (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            pulser_integral_range.first,
            pulser_integral_range.second);
    profiles["Pulser"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Pulser"]->SetStats(false);

    hName = TString::Format("h_Pulser_ProfilePedestal_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Profile Pedestal (%5.1f - %5.1f); event number / 5000events; signal/mV",
            _sensor.c_str(),_id,
            pedestal_integral_range.first,
            pedestal_integral_range.second);
    profiles["Pulser_Pedestal"] = new TProfile(hName,hTitle,1,0,1000);
    profiles["Pulser_Pedestal"]->SetStats(false);

    // hName = TString::Format("h_Pulser_ProfileDeltaIntegral_%s_%d",_sensor.c_str(),_id);
    // hTitle = TString::Format("%s %d: Profile Signal-PedestalIntegral; event number / 5000events; signal/mV",_sensor.c_str(),_id);
    // profiles["Pulser_DeltaIntegral"] = new TProfile(hName,hTitle,1,0,1000);
    // profiles["Pulser_DeltaIntegral"]->SetStats(false);
}

//=====================================================================
//======== waveform stacks   ==========================================
//=====================================================================
void WaveformHistos::InitWaveformStacks(){
    TString hName = TString::Format("h_wf_stack_%s_%d",_sensor.c_str(),_id);
    TString hTitle = TString::Format("%s %d: Waveform Stack;time; signal/mV",_sensor.c_str(),_id);
    h_wf_stack = new THStack(hName,hTitle);

    hName = TString::Format("h_goodtwf_stack_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: Good Waveform Stack;time; signal/mV",_sensor.c_str(),_id);
    h_goodwf_stack= new THStack(hName,hTitle);

    hName = TString::Format("h_badfftwf_stack_%s_%d",_sensor.c_str(),_id);
    hTitle = TString::Format("%s %d: BadFFT Waveform Stack;time; signal/mV",_sensor.c_str(),_id);
    h_badfftwf_stack = new THStack(hName,hTitle);

    for (int i = 0; i < _n_wfs; i++){
        hName = TString::Format("Waveform_%d_%d",_id,i);
        hTitle = TString::Format("Waveform ID %d - %d",_id,i);
        _Waveforms      .push_back(new TH1F(hName,hTitle,_n_samples,0,_n_samples));
        if (_Waveforms.back()->GetXaxis())
            _Waveforms.back()->GetXaxis()->SetTitle("n");
        if (_Waveforms.back()->GetYaxis())
            _Waveforms.back()->GetYaxis()->SetTitle("Signal / mV");
        h_wf_stack->Add(_Waveforms.back());

        hName = TString::Format("BadWaveform_%d_%d",_id,i);
        hTitle = TString::Format("BadWaveform ID %d - %d",_id,i);
        _BadFFTWaveforms.push_back(new TH1F(hName      ,hTitle      ,_n_samples,0,_n_samples));
        if (_BadFFTWaveforms.back()->GetXaxis())
            _BadFFTWaveforms.back()->GetXaxis()->SetTitle("n");
        if (_BadFFTWaveforms.back()->GetYaxis())
            _BadFFTWaveforms.back()->GetYaxis()->SetTitle("Signal / mV");
        h_badfftwf_stack->Add(_BadFFTWaveforms.back());

        hName = TString::Format("GoodWaveform_%d_%d",_id,i);
        hTitle = TString::Format("GoodWaveform ID %d - %d",_id,i);
        _GoodWaveforms      .push_back(new TH1F(hName,hTitle,_n_samples,0,_n_samples));
        if (_GoodWaveforms.back()->GetXaxis())
            _GoodWaveforms.back()->GetXaxis()->SetTitle("n");
        if (_GoodWaveforms.back()->GetYaxis())
            _GoodWaveforms.back()->GetYaxis()->SetTitle("Signal / mV");
        h_goodwf_stack->Add(_GoodWaveforms.back());
    }

    for (std::map<std::string, TH1*>::iterator it = profiles.begin();it!=profiles.end();it++){
        it->second->SetMinimum(0);
    }
}

void WaveformHistos::Reinitialize_Waveforms() {
    //	if (h_wf_stack){
    //		delete h_wf_stack;
    //	}
    cout<<"WaveformHistos::Reinitialize_Waveforms of "<<_sensor.c_str()<<"_"<<_id<<" with "<<getNSamples()<<" Samples."<<endl;
    //	TString hName = TString::Format("h_wf_stack_%s_%d",_sensor.c_str(),_id);
    //	TString hTitle = TString::Format("%s %d: Waveform Stack;time; signal/mV",_sensor.c_str(),_id);
    ////	h_wf_stack = new THStack(hName,hTitle);
    for (int i = 0; i < _n_wfs; i++){
        //		hName = TString::Format("Waveform_%d_%d",_id,i);
        //		hTitle = TString::Format("Waveform ID %d - %d",_id,i);
        TH1F* histo = _Waveforms.at(i);
        histo->SetBins(_n_samples,0,_n_samples);
    }

}

void WaveformHistos::Reinitialize_BadFFTWaveforms() {
    cout<<"WaveformHistos::Reinitialize_BadFFTWaveforms of "<<_sensor.c_str()<<"_"<<_id<<" with "<<getNSamples()<<" Samples."<<endl;
    for (int i = 0; i < _n_wfs; i++){
        TH1F* histo = _BadFFTWaveforms.at(i);
        histo->SetBins(_n_samples,0,_n_samples);
    }
}

void WaveformHistos::Reinitialize_GoodWaveforms() {
    cout<<"WaveformHistos::Reinitialize_GoodWaveforms of "<<_sensor.c_str()<<"_"<<_id<<" with "<<getNSamples()<<" Samples."<<endl;
    for (int i = 0; i < _n_wfs; i++){
        TH1F* histo = _GoodWaveforms.at(i);
        histo->SetBins(_n_samples,0,_n_samples);
    }
}
void WaveformHistos::Fill(const SimpleStandardWaveform & wf)
{
//    std::cout<<"WaveformHistos::Fill"<<std::endl;
    if (wf.getNSamples() > this->getNSamples()){
        _n_samples = wf.getNSamples();
        Reinitialize_Waveforms();
        Reinitialize_BadFFTWaveforms();
        Reinitialize_GoodWaveforms();
    }
    bool isPulserEvent = wf.isPulserEvent();
    this->FillEvent(wf, isPulserEvent);
    // if (isPulserEvent)
    //     this->FillPulserEvent(wf);
    // else
    //     this->FillSignalEvent(wf);
}
void WaveformHistos::FillEvent(const SimpleStandardWaveform & wf, bool isPulserEvent){
//    std::cout<<"WaveformHistos::FillSignalEvent "<<std::endl;
    int event_no = wf.getEvent();
    ULong64_t timestamp = wf.getTimestamp();
    int sign = wf.getSign(); //why is this here? it's never properly assigned
    EventCategroy cat = GOOD_EVENT;
    float maxSpread   = wf.maxSpreadInRegion(200,400);
    // do not record events with a flat line due to leakage current
//    bool bFlatlineEvent = false;
    if(maxSpread < 10)
        cat = FLAT_EVENT;
    //if ((wf.getMeanFFT() > 500 ) )
    if ((wf.getMeanFFT() > 1000 ) ) //Dmitry
        cat = BAD_FFT_MEAN_EVENT;
    //if   ( (1./wf.getMaxFFT()) < 1E-4 ){
    if   ( (1./wf.getMaxFFT()) < 3E-5 ){ //Dmitry
        if (cat == BAD_FFT_MEAN_EVENT)
            cat = BAD_FFT_BOTH_EVENT;
        else
            cat = BAD_FFT_MAX_EVENT;
    }
    bool failsFFTCuts = ( (cat == BAD_FFT_MAX_EVENT)|| (cat == BAD_FFT_MEAN_EVENT) || (cat == BAD_FFT_BOTH_EVENT));
    float maxfft = (wf.getMaxFFT() ? wf.getMaxFFT() : 1000000.);
    if (isPulserEvent)
        cat = PULSER_EVENT;
    histos["nFlatLineEvents"]->Fill((bool)(cat == FLAT_EVENT));
    histos["nBadFFTEvents"]->Fill(failsFFTCuts);
    histos["CategoryVsEvent"]->Fill(event_no,(int)cat);
    histos["Category"]->Fill((int)cat);
    if (cat == FLAT_EVENT)
            return;
    // check if the event passes/fails the FFT cuts

    // if (!(event_no%1000))
    //     cout << "ev " << event_no << " in wf " << wf.getChannelName() << " this is the mean FFT: " << wf.getMeanFFT() <<
    //     "   this is the inv. max: " << 1./wf.getMaxFFT() << "   at time " << timestamp << endl;

    float min      = wf.getMin();
    float max      = wf.getMax();
    float delta    = fabs(max-min);
    float integral = wf.getIntegral();
    float signalSpread   = wf.maxSpreadInRegion(signal_integral_range.first  ,signal_integral_range.second);
    float pedestalSpread = wf.maxSpreadInRegion(pedestal_integral_range.first,pedestal_integral_range.second);
    float pulserSpread = wf.maxSpreadInRegion(pulser_integral_range.first,pulser_integral_range.second);


    float signal_maximum    = wf.getMaximum (signal_integral_range.first  ,signal_integral_range.second);
    float signal_minimum    = wf.getMinimum (signal_integral_range.first  ,signal_integral_range.second);

    float signal_integral   = wf.getIntegral(signal_integral_range.first  ,signal_integral_range.second);
    float pedestal_integral = wf.getIntegral(pedestal_integral_range.first,pedestal_integral_range.second);
    float pulser_integral   = wf.getIntegral(pulser_integral_range.first,pulser_integral_range.second);
    ULong64_t time_stamp = wf.getTimestamp();

    std::map<std::string, TH1*>::iterator it ;
    if (time_start == 0){
        time_start = time_stamp;
        float timestamp = (float)time_stamp/1e7;
        TDatime time = timestamp;
        TString hTitle = (TString)"time/s   -  Start Time: "+time.AsString();
        for (it = time_profiles.begin();it!=time_profiles.end();it++)
            it->second->GetXaxis()->SetTitle(hTitle);
        std::cout<<event_no<<":Setting Timestamp to: "<<time_start<<" "<<timestamp<<endl;
    }
    string prefix = "";
    if (isPulserEvent) prefix = "Pulser_";
    if (failsFFTCuts)  prefix = "BadFFT_";

    histos["PulserEvents"]->Fill(isPulserEvent);
    histos["BadFFTEvents"]->Fill(failsFFTCuts);
    histos["SignalEvents"]->Fill((!failsFFTCuts) && !(isPulserEvent));
    histos[prefix+"MeanFFT"]     ->Fill(wf.getMeanFFT()   );
    histos[prefix+"InvMaxFFT"]   ->Fill(1./maxfft );
    histos[prefix+"Signal"]     ->Fill(signalSpread);
    for (it = profiles.begin();it!=profiles.end();it++){
        if (it->second->GetXaxis()->GetXmax() < event_no){
            int bins = (event_no+5000)/5000;
            int max = (bins)*5000;
            it->second->SetBins(bins,0,max);
            //			cout<<it->first<<": Extend Profile "<<bins<<" "<<max<<endl;
        }
        if     (it->first == "SignalEvents")
            it->second->Fill(event_no,!(isPulserEvent || failsFFTCuts));
        else if(it->first == "BadFFTEvents")
            it->second->Fill(event_no,failsFFTCuts);
        else if(it->first == "PulserEvents")
            it->second->Fill(event_no,isPulserEvent);
    }

    if (!failsFFTCuts){
        histos[prefix+"FullAverage"]->Fill(sign*integral);
        histos[prefix+"Pedestal"]   ->Fill(pedestalSpread);
        histos[prefix+"Pulser"]   ->Fill(pulserSpread);
        histos[prefix+"SignalMinusPedestal"] -> Fill(signalSpread-pedestalSpread);
        histos[prefix+"PulserMinusPedestal"] -> Fill(signalSpread-pedestalSpread);

        histos[prefix+"SignalIntegral"]     ->Fill(sign*signal_integral);
        histos[prefix+"PedestalIntegral"]   ->Fill(sign*pedestal_integral);
        histos[prefix+"PulserIntegral"]     ->Fill(sign*pulser_integral);

        for (it = time_profiles.begin();it!=time_profiles.end();it++){
            float delta_t = (time_stamp-time_start)/1e7;
//            std::cout<<event_no<<"delta_t: "<<delta_t<<std::endl;
            if (it->second->GetXaxis()->GetXmax() < delta_t){
                int t_bin = 10;
                int bins = (delta_t+t_bin)/t_bin;
                int max = (bins)*t_bin;
                it->second->SetBins(bins,0,max);
            }
                time_profiles[prefix+"SignalMinusPedestal"]->Fill(delta_t,signalSpread-pedestalSpread);
        }
        for (it = profiles.begin();it!=profiles.end();it++){
            if (it->second->GetXaxis()->GetXmax() < event_no){
                int bins = (event_no+5000)/5000;
                int max = (bins)*5000;
                it->second->SetBins(bins,0,max);
                //			cout<<it->first<<": Extend Profile "<<bins<<" "<<max<<endl;
            }
            if (it->first == prefix+"FullIntegral")
                it->second->Fill(event_no,sign*integral);
            else if (it->first == prefix+"Signal")
                it->second->Fill(event_no,signalSpread);
            else if (it->first == prefix+"Pedestal")
                it->second->Fill(event_no,pedestalSpread);
            else if (it->first == prefix+"Pulser")
                it->second->Fill(event_no,pulserSpread);
            else if (it->first == prefix+"SignalMinusPedestal")
                it->second->Fill(event_no,signalSpread-pedestalSpread);
            if (do_fitting){
                if (event_no % 5000 == 0 && event_no >20000){
                    TF1* fit = new TF1("expoFit", "pol0(0)+expo(1)",0,event_no+5000);
                    it->second->Fit(fit,"Q");
                    delete fit;
                }
            }
        }
    }

    UpdateRanges();
    // all waveforms
    TH1F* gr = _Waveforms[n_fills%_n_wfs];
    for (int n = 0; n < wf.getNSamples();n++)
        gr->SetBinContent(n+1,wf.getData()[n]);
    for (int i = 0; i < _n_wfs; i++)
        _Waveforms[(n_fills-i)%_n_wfs]->SetLineColor(kAzure+i);
    gr->SetEntries(event_no);
    n_fills++;
    if (n_fills<=1){
        //      gr->Draw("APL");
        if (gr->GetXaxis())
            gr->GetXaxis()->SetTitle("n");
        if (gr->GetYaxis())
            gr->GetYaxis()->SetTitle("Signal / mV");
    }
    //good waveforms
    if(!failsFFTCuts) {
        gr = _GoodWaveforms[n_fills_good%_n_wfs];
        for (int n = 0; n < wf.getNSamples();n++)
            gr->SetBinContent(n+1,wf.getData()[n]);
        for (int i = 0; i < _n_wfs; i++)
            _GoodWaveforms[(n_fills_good-i)%_n_wfs]->SetLineColor(kAzure+i);
        gr->SetEntries(event_no);
        n_fills_good++;
        if (n_fills_good<=1){
            if (gr->GetXaxis())
                gr->GetXaxis()->SetTitle("n");
            if (gr->GetYaxis())
                gr->GetYaxis()->SetTitle("Signal / mV");
        }
    }
    // bad ffts
    else if(failsFFTCuts && !isPulserEvent) {
        gr = _BadFFTWaveforms[n_fills_bad%_n_wfs];
        for (int n = 0; n < wf.getNSamples();n++)
            gr->SetBinContent(n+1,wf.getData()[n]);
        for (int i = 0; i < _n_wfs; i++)
            _BadFFTWaveforms[(n_fills_bad-i)%_n_wfs]->SetLineColor(kAzure+i);
        gr->SetEntries(event_no);
        n_fills_bad++;

        if (n_fills_bad<=1){
            //		gr->Draw("APL");
            if (gr->GetXaxis())
                gr->GetXaxis()->SetTitle("n");
            if (gr->GetYaxis())
                gr->GetYaxis()->SetTitle("Signal / mV");
        }
    }
}

void WaveformHistos::Reset() {
    for (int i = 0; i < _Waveforms.size(); i++)
        _Waveforms[i]->Reset();
    for (int i = 0; i < _BadFFTWaveforms.size(); i++)
        _BadFFTWaveforms[i]->Reset();
    // we have to reset the aux array as well
    std::map<std::string, TH1*>::iterator it;
    for (it = histos.begin(); it != histos.end();it++)
        it->second->Reset();
    for (it = profiles.begin(); it != profiles.end();it++)
        it->second->Reset();

    n_fills = 0;
    n_fills_bad = 0;
    n_fills_good = 0;
}

void WaveformHistos::Calculate(const int currentEventNum)
{
    _wait = true;
    //	cout<<"WaveformHistos::Calculate"<<currentEventNum<<endl;
    _wait = false;
}


void WaveformHistos::Write()
{

    for (UInt_t i = 0; i< _Waveforms.size();i++)
        _Waveforms.at(i)->Write();
    h_wf_stack->Write();
    h_badfftwf_stack->Write();
    std::map<std::string, TH1*>::iterator it;
    for (it = profiles.begin();it!=profiles.end();it++)
        it->second->Write();
    for(it = histos.begin();it!=histos.end();it++)
        it->second->Write();
}

int WaveformHistos::SetHistoAxisLabelx(TH1* histo,string xlabel)
{
    if (histo!=NULL)
    {
        histo->GetXaxis()->SetTitle(xlabel.c_str());
    }
    return 0;
}

void WaveformHistos::SetPedestalIntegralRange(float min, float max) {
    pedestal_integral_range = make_pair(min,max);
    TString hTitle = TString::Format("%s %d: Profile PedestalIntegral (%5.1f - %5.1f)",
            _sensor.c_str(),_id,min,max);
    profiles["PedestalIntegral"]->SetTitle(hTitle);
}

void WaveformHistos::SetSignalIntegralRange(float min, float max) {
    signal_integral_range = make_pair(min,max);
    TString hTitle = TString::Format("%s %d: Profile PedestalIntegral (%5.1f - %5.1f)",
            _sensor.c_str(),_id,min,max);
    profiles["Signal"]->SetTitle(hTitle);
}

TProfile* WaveformHistos::getProfile(std::string key) const {
//    std::cout<<"WaveformHistos::getProfile "<<key<<std::endl;
    std::map<std::string, TH1*>::const_iterator it = profiles.find(key);
    if ( it == profiles.end()) return 0;
    return (TProfile*)profiles.at(key);
}

TH1* WaveformHistos::getHisto(std::string key) const {
//    std::cout<<"WaveformHistos::getHisto"<<key<<std::endl;
    std::map<std::string, TH1*>::const_iterator it = histos.find(key);
    if ( it == histos.end()) return 0;
    return histos.at(key);
}

TProfile* WaveformHistos::getTimeProfile(std::string key) const {
//    std::cout<<"WaveformHistos::getHisto"<<key<<std::endl;
    std::map<std::string, TH1*>::const_iterator it = time_profiles.find(key);
    if ( it == time_profiles.end()) return 0;
    return (TProfile*)time_profiles.at(key);
}

int WaveformHistos::SetHistoAxisLabels(TH1* histo,string xlabel, string ylabel)
{
    SetHistoAxisLabelx(histo,xlabel);
    SetHistoAxisLabely(histo,ylabel);
    return 0;
}



int WaveformHistos::SetHistoAxisLabely(TH1* histo,string ylabel)
{
    if (histo!=NULL)
    {
        histo->GetYaxis()->SetTitle(ylabel.c_str());
    }
    return 0;
}

void WaveformHistos::SetMaxRangeX(std::string name, float min, float max) {
    if (min<max)
        rangesX[name] = make_pair(min,max);
    else
        std::cout<<"Cannot SetMaxRangeY: "<<name<< " to "<<min<<max<<std::endl;
}

void WaveformHistos::SetMaxRangeY(std::string name, float min, float max) {
    if (min<max)
        rangesY[name] = make_pair(min,max);
    else
        std::cout<<"Cannot SetMaxRangeY: "<<name<< " to "<<min<<max<<std::endl;
}


void WaveformHistos::UpdateRanges() {
    std::map<std::string, TH1*>::iterator it;
    for (it = histos.begin(); it != histos.end();it++)
        UpdateRange(it->second);

    float min = _Waveforms[0]->GetBinContent(_Waveforms[0]->GetMinimumBin());
    float max = _Waveforms[0]->GetBinContent(_Waveforms[0]->GetMaximumBin());
    bool changed = false;
    if (min < min_wf){
        this->min_wf = min;
        changed = true;
    }
    else
        min = min_wf;
    if (max > max_wf){
        this->max_wf = max;
        changed = true;
    }
    else
        max = max_wf;
    if (changed){
        float delta = max - min;
        min = min - .05 * delta;
        max = max + .05 * delta;
        //	cout<<"Range: "<<min<<" "<<max<<endl;
        _Waveforms[0]->GetYaxis()->SetRangeUser(min,max);
    }

}

void WaveformHistos::UpdateRange(TH1* histo) {
    //	cout<<"Update range for "<<histo->GetName()<<endl;
    int binLow = histo->FindFirstBinAbove();
    int binHigh = histo->FindLastBinAbove();
    int delta = binHigh-binLow;
    binLow -= int(.05*delta+1);
    binHigh += int(.05*delta+1);
    binLow = binLow<1?1:binLow;
    binHigh = histo->GetNbinsX()<binHigh?histo->GetNbinsX():binHigh;
    std::string name = histo->GetName();
    if (rangesX.find(name) != rangesX.end()){
        pair<float, float> range = rangesX.find(name)->second;
        //		std::cout<<"  Max range of "<<histo->GetName()<<" "<<range.first<<"-"<<range.second<<std::endl;
        //		std::cout<<"  entries in "<<histo->GetXaxis()->GetBinLowEdge(binLow)<<"-"<<histo->GetXaxis()->GetBinUpEdge(binHigh)<<std::endl;
        if (binLow < histo->GetXaxis()->FindBin(range.first))
            binLow = histo->GetXaxis()->FindBin(range.first);
        if (binHigh > histo->GetXaxis()->FindBin(range.second))
            binHigh = histo->GetXaxis()->FindBin(range.second);
        //		std::cout<<"  ==> new range in "<<histo->GetXaxis()->GetBinLowEdge(binLow)<<"-"<<histo->GetXaxis()->GetBinUpEdge(binHigh)<<std::endl;
    }
    histo->GetXaxis()->SetRange(binLow,binHigh);
}

