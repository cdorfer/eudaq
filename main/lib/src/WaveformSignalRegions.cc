/*
 * WaveformSignalRegions.cpp
 *
 *  Created on: Oct 14, 2015
 *      Author: bachmair
 */

#include <eudaq/WaveformSignalRegions.hh>

WaveformSignalRegions::WaveformSignalRegions(int channel, signed char pol) {
    std::cout<<"New WaveformSignalRegions "<<channel<<" "<<(int)pol<<std::flush;
    this->channel=channel;
    polarity=pol;
    std::cout<<"."<<std::endl;
}


void WaveformSignalRegions::AddRegion(WaveformSignalRegion region) {
//    std::cout<<"AddRegion: "<<region<<std::endl;
    region.SetPolarity(polarity);
    this->regions.push_back(region);
}

void WaveformSignalRegions::Print(std::ostream& out) const {
    for (auto i: this->regions)
        out<<"\t"<<i<<"\n";
}

void WaveformSignalRegions::Reset(){
    for (auto i: this->regions)
            i.ResetIntegrals();
}


WaveformSignalRegion* WaveformSignalRegions::GetRegion(UInt_t i) {
    if (i<regions.size())
        return &(regions[i]);
    return 0;
}
