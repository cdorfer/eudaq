/*
 * EUDAQMonitorCollection.cpp
 *
 *  Created on: Sep 27, 2011
 *      Author: stanitz
 */

#include "../include/EUDAQMonitorCollection.hh"
#include "OnlineMon.hh"

EUDAQMonitorCollection::EUDAQMonitorCollection(): BaseCollection()
{
  mymonhistos=NULL;
  histos_init=false;
  cout << " Initialising EUDAQMonitor Collection" <<endl;
  CollectionType=EUDAQMONITOR_COLLECTION_TYPE;

}

EUDAQMonitorCollection::~EUDAQMonitorCollection()
{
  // TODO Auto-generated destructor stub
}

void EUDAQMonitorCollection::fillHistograms(const SimpleStandardEvent & ev)
{
  Fill(ev);
}


void EUDAQMonitorCollection::Reset()
{
  if(mymonhistos != NULL) mymonhistos->Reset();
}



void EUDAQMonitorCollection::Write(TFile *file)
{
  if (file==NULL)
  {
    cout << "EUDAQMonitorCollection::Write File pointer is NULL"<<endl;
    exit(-1);
  }
  if (gDirectory!=NULL) //check if this pointer exists
  {
    gDirectory->mkdir("EUDAQMonitor");
    gDirectory->cd("EUDAQMonitor");
    mymonhistos->Write();
    gDirectory->cd("..");
  }
}

void EUDAQMonitorCollection::bookHistograms(const SimpleStandardEvent & /*simpev*/)
{
  if (_mon != NULL)
  {
    cout << "EUDAQMonitorCollection:: Monitor running in online-mode" << endl;
    string performance_folder_name="EUDAQ Monitor";
    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Number of Planes"));
    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Number of Planes"),mymonhistos->getPlanes_perEventHisto());
    cout<<"Add Number of Waveforms:" << endl;
    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Number of Waveforms"));
    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Number of Waveforms"),mymonhistos->getWaveforms_perEventHisto());
    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Hits vs. Plane"));
    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Hits vs. Plane"),mymonhistos->getHits_vs_PlaneHisto());
    _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Hits vs. Event"));
    _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Hits vs. Event"),mymonhistos->getHits_vs_EventsTotal());
    if(_mon->getUseTrack_corr())
    {
      _mon->getOnlineMon()->registerTreeItem((performance_folder_name+"/Tracks per Event"));
      _mon->getOnlineMon()->registerHisto( (performance_folder_name+"/Tracks per Event"),mymonhistos->getTracksPerEventHisto() );
    }

    _mon->getOnlineMon()->makeTreeItemSummary(performance_folder_name.c_str()); //make summary page

    string name_root=performance_folder_name+"/Planes";
    for (unsigned int i=0; i<mymonhistos->getNplanes(); i++)
    {
      stringstream namestring_hits;
      stringstream namestring_tlu;
      namestring_hits<<name_root<<"/Hits Sensor Plane "<<i;
      namestring_tlu<<name_root<<"/TLU Delta Sensor Plane "<<i;
      cout<<"Register: "<<namestring_hits.str()<<endl;
      _mon->getOnlineMon()->registerTreeItem(namestring_hits.str());
      cout<<"Register: "<<namestring_hits.str()<<endl;
      _mon->getOnlineMon()->registerHisto(namestring_hits.str(),mymonhistos->getHits_vs_Events(i));
      cout<<"Register: "<<namestring_tlu.str()<<endl;
      _mon->getOnlineMon()->registerTreeItem(namestring_tlu.str());
      cout<<"Register: "<<namestring_tlu.str()<<endl;
      _mon->getOnlineMon()->registerHisto(namestring_tlu.str(),mymonhistos->getTLUdelta_perEventHisto(i));
      stringstream namestring_triggerphase;
      namestring_triggerphase<<name_root<<"/Trigger Phase Vs EventNo"<<i;
      _mon->getOnlineMon()->registerTreeItem(namestring_triggerphase.str());
      cout<<"Register: "<<namestring_triggerphase.str()<<endl;
      _mon->getOnlineMon()->registerHisto(namestring_triggerphase.str(),mymonhistos->getTriggerPhase_vs_Events(i),"COLZ");
      namestring_triggerphase.str("");
      namestring_triggerphase.clear();

      namestring_triggerphase<<name_root<<"/Trigger Phase "<<i;
      _mon->getOnlineMon()->registerTreeItem(namestring_triggerphase.str());
      cout<<"Register: "<<namestring_triggerphase.str()<<endl;
      _mon->getOnlineMon()->registerHisto(namestring_triggerphase.str(),mymonhistos->getTriggerPhaseHisto(i),"");

    } //end plane loop
    _mon->getOnlineMon()->makeTreeItemSummary(name_root.c_str()); //make summary page


    name_root=performance_folder_name+"Waveforms";
    for (unsigned int i=0; i<mymonhistos->getNwaveforms(); i++)
    {
      stringstream namestring_raw;
      stringstream namestring_amp;
      namestring_raw<<name_root<<"/Raw Waveforms "<<i;
      namestring_amp<<name_root<<"/Amplitude Difference"<<i;
//      _mon->getOnlineMon()->registerTreeItem(namestring_amp.str());
//      _mon->getOnlineMon()->registerHisto(namestring_amp.str(),mymonhistos->getWaveforms_AmplitudeHisto(i));
//      _mon->getOnlineMon()->registerGraph(namestring_raw.str(),new TGraph(0),"APL",0);
    } //end waveform loop
    _mon->getOnlineMon()->makeTreeItemSummary(name_root.c_str()); //make summary page
  }// end if (_mon != NULL)
}

EUDAQMonitorHistos *EUDAQMonitorCollection::getEUDAQMonitorHistos()
{
  return mymonhistos;
}



void EUDAQMonitorCollection::Calculate(const unsigned int /*currentEventNumber*/)
{

}



void EUDAQMonitorCollection::Fill(const SimpleStandardEvent & simpev)
{
  if (histos_init==false)
  {
    mymonhistos=new EUDAQMonitorHistos(simpev);
    if (mymonhistos==NULL)
    {
      cout << "EUDAQMonitorCollection:: Can't book histograms " <<endl;
      exit(-1);
    }
    bookHistograms(simpev);
    cout<<"histos_init:"<<histos_init<<endl;
    histos_init=true;
  }
  mymonhistos->Fill(simpev);
}


