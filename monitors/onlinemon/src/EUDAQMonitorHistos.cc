/*
 * EUDAQMonitorHistos.cc
 *
 *  Created on: Sep 27, 2011
 *      Author: stanitz
 */

#include "../include/EUDAQMonitorHistos.hh"

EUDAQMonitorHistos::EUDAQMonitorHistos(const SimpleStandardEvent &ev)
{
  nplanes=ev.getNPlanes();
  nwfs = ev.getNWaveforms();
  Hits_vs_Events=new TProfile*[nplanes];
  TLUdelta_perEventHisto=new TProfile*[nplanes];

//  TriggerPhasePerEvent.resize(nplanes,0);

  Planes_perEventHisto= new TH1F("Planes in Event","Planes in Event",nplanes*2,0,nplanes*2);
  Waveforms_perEventHisto= new TH1F("Waveforms in Event","Waveforms in Event",nwfs*2,0,nwfs*2);
  Hits_vs_PlaneHisto =new TProfile("Hits vs Plane", "Hits vs Plane",nplanes,0, nplanes);
  Hits_vs_EventsTotal=new TProfile("Hits vs Event", "Hits vs Event",1000, 0, 20000);
  //    TracksPerEvent = new TH2I("Tracks per Event", "Tracks per Event", 1000, 0, 20000, 5, 0, 5);
  TracksPerEvent = new TProfile("Tracks per Event", "Tracks per Event", 1000, 0, 20000);

  Hits_vs_EventsTotal->SetBit(TH1::kCanRebin);
  TracksPerEvent->SetBit(TH1::kCanRebin);

  for (unsigned int i=0; i<nplanes; i++)
  {

    stringstream number;
    stringstream histolabel;
    stringstream histolabel_tlu;
    number<<i;
    string name=ev.getPlane(i).getName()+" "+number.str();
    Hits_vs_PlaneHisto->GetXaxis()->SetBinLabel(i+1,name.c_str());

    histolabel<< "Hits vs Event Nr "<< name;
    histolabel_tlu<<"TLU Delta vs Event Nr " << name;

    Hits_vs_Events[i]=new TProfile(histolabel.str().c_str(), histolabel.str().c_str(),1000,0, 20000);
    TLUdelta_perEventHisto[i]=new TProfile(histolabel_tlu.str().c_str(), histolabel_tlu.str().c_str(),1000,0, 20000);

    TString hName = "hTriggerPhasePerEvent_"+(TString)ev.getPlane(i).getName()+"_"+number.str();
    TString hTitle = "Trigger Phase vs Event No. "+(TString) name;
    hTitle+=";event no;Trigger Phase";
    std::cout<<i<<" "<<hName<<" "<<hTitle<<std::endl;
    TriggerPhasePerEvent.push_back( new TH2D(hName,hTitle,1000,0,20000,10,0,10));
    TriggerPhasePerEvent.back()->SetStats(false);
    std::cout<<TriggerPhasePerEvent.at(i)<<std::endl;

    hName = "hTriggerPhase_"+(TString)ev.getPlane(i).getName()+"_"+number.str();
    hTitle = "Trigger Phase"+(TString) name;
    hTitle+=";Trigger Phase;event no";
    std::cout<<i<<" "<<hName<<" "<<hTitle<<std::endl;
    TriggerPhaseHisto.push_back( new TH1D(hName,hTitle,10,0,10));
    TriggerPhaseHisto.back()->SetStats(false);
    std::cout<<TriggerPhaseHisto.at(i)<<std::endl;

    Hits_vs_Events[i]->SetLineColor(i+1);
    Hits_vs_Events[i]->SetMarkerColor(i+1);
    TLUdelta_perEventHisto[i]->SetLineColor(i+1);
    TLUdelta_perEventHisto[i]->SetMarkerColor(i+1);


    //fix for root being stupid
    if (i==9) //root features //FIXME
    {
      Hits_vs_Events[i]->SetLineColor(i+2);
      Hits_vs_Events[i]->SetMarkerColor(i+2);
      TLUdelta_perEventHisto[i]->SetLineColor(i+2);
      TLUdelta_perEventHisto[i]->SetMarkerColor(i+2);
    }
    Hits_vs_Events[i]->SetBit(TH1::kCanRebin);
    TriggerPhasePerEvent[i]->SetBit(TH1::kCanRebin);
    TLUdelta_perEventHisto[i]->SetBit(TH1::kCanRebin);
  }

}
\
EUDAQMonitorHistos::~EUDAQMonitorHistos()
{
  // TODO Auto-generated destructor stub
}

void EUDAQMonitorHistos::Fill(const SimpleStandardEvent &ev)
{
  unsigned int event_nr=ev.getEvent_number();
  Planes_perEventHisto->Fill(ev.getNPlanes());
  Waveforms_perEventHisto->Fill(ev.getNWaveforms());
  unsigned int nhits_total=0;

  for (unsigned int i=0; i<nplanes; i++)
  {
    Hits_vs_PlaneHisto->Fill(i,ev.getPlane(i).getNHits());
    Hits_vs_Events[i]->Fill(event_nr,ev.getPlane(i).getNHits());
    TLUdelta_perEventHisto[i]->Fill(event_nr,ev.getPlane(i).getTLUEvent()-(event_nr%32768));// TLU counter can only hnadel 32768 counts
    nhits_total+=ev.getPlane(i).getNHits();
//    if (TriggerPhasePerEvent[i]->GetXaxis()->GetXmax() < event_nr){
//        int bins = (event_nr+2000)/2000;
//        int max = (bins+2)*2000;
//        TriggerPhasePerEvent[i]->SetBins(bins,0,max,10,0,10);
//        cout<<TriggerPhasePerEvent[i]<<": Extend Profile "<<bins<<" "<<max<<endl;
//    }
    TriggerPhaseHisto[i]->Fill(ev.getPlane(i).getTriggerPhase());
    (TriggerPhasePerEvent[i])->Fill(event_nr,ev.getPlane(i).getTriggerPhase());
  }
  Hits_vs_EventsTotal->Fill(event_nr,nhits_total);
}

void EUDAQMonitorHistos::Fill(const unsigned int evt_number, const unsigned int tracks)
{
  TracksPerEvent->Fill(evt_number, tracks);
}



void EUDAQMonitorHistos::Write()
{
  Planes_perEventHisto->Write();
  Waveforms_perEventHisto->Write();
  Hits_vs_PlaneHisto->Write();
  for (unsigned int i=0; i<nplanes; i++)
  {
    Hits_vs_Events[i]->Write();
    TLUdelta_perEventHisto[i]->Write();
    TriggerPhasePerEvent[i]->Write();
    TriggerPhaseHisto[i]->Write();
  }
  Hits_vs_EventsTotal->Write();
  TracksPerEvent->Write();
}



TProfile *EUDAQMonitorHistos::getHits_vs_Events(unsigned int i) const
{
  return Hits_vs_Events[i];
}

TProfile *EUDAQMonitorHistos::getHits_vs_EventsTotal() const
{
  return Hits_vs_EventsTotal;
}

TProfile *EUDAQMonitorHistos::getHits_vs_PlaneHisto() const
{
  return Hits_vs_PlaneHisto;
}

TH1F *EUDAQMonitorHistos::getPlanes_perEventHisto() const
{
  return Planes_perEventHisto;
}

TH1F *EUDAQMonitorHistos::getWaveforms_perEventHisto() const
{
  return Waveforms_perEventHisto;
}

TProfile *EUDAQMonitorHistos::getTLUdelta_perEventHisto(unsigned int i) const
{
  return TLUdelta_perEventHisto[i];
}

//TH2I *EUDAQMonitorHistos::getTracksPerEventHisto() const
TProfile *EUDAQMonitorHistos::getTracksPerEventHisto() const
{
  return TracksPerEvent;
}

unsigned int EUDAQMonitorHistos::getNplanes() const
{
  return nplanes;
}

TH1F* EUDAQMonitorHistos::getWaveforms_AmplitudeHisto(unsigned int i) const {
	return 0;
}

TH2D* EUDAQMonitorHistos::getTriggerPhase_vs_Events(unsigned int i) const {
    return TriggerPhasePerEvent.at(i);
}

unsigned int EUDAQMonitorHistos::getNwaveforms() const
{
  return nwfs;
}

void EUDAQMonitorHistos::setPlanes_perEventHisto(TH1F *Planes_perEventHisto)
{
  this->Planes_perEventHisto = Planes_perEventHisto;
}

void EUDAQMonitorHistos::setWaveforms_perEventHisto(TH1F *Waveforms_perEventHisto)
{
  this->Waveforms_perEventHisto = Waveforms_perEventHisto;
}

void EUDAQMonitorHistos::Reset()
{
  Planes_perEventHisto->Reset();
  Waveforms_perEventHisto->Reset();
  Hits_vs_PlaneHisto->Reset();
  for (unsigned int i=0; i<nplanes; i++)
  {
    Hits_vs_Events[i]->Reset();
    TLUdelta_perEventHisto[i]->Reset();
  }
  Hits_vs_EventsTotal->Reset();
  TracksPerEvent->Reset();
}


