/*
 * WaveformSignalRegion.cc
 *
 *  Created on: Oct 13, 2015
 *      Author: bachmair
 */

#include <eudaq/WaveformSignalRegion.hh>

//ClassImp(WaveformSignalRegion);
WaveformSignalRegion::WaveformSignalRegion(int low, int high, std::string name):peak_pos(-1) {
    // TODO Auto-generated constructor stub
    this->low_border = low;
    this->high_border = high;
    this->polarity = 0;
    this->name=name;
}

WaveformSignalRegion::~WaveformSignalRegion() {
    // TODO Auto-generated destructor stub
}

void WaveformSignalRegion::ResetIntegrals() {
    for (UInt_t i = 0; i<this->integral_names.size();i++){
        WaveformIntegral* p = this->GetIntegralPointer(i);
        p->Reset();
    }
    peak_pos = -1;
}

void WaveformSignalRegion::Print(std::ostream& out) const {
    out<<"WaveformSignalRegion: \""<<name<<"\": "<<low_border<<"-"<<high_border<<" for Polarity: "<<(int)polarity;
    out<<" with "<<integrals.size()<<" Integrals";
    if (peak_pos>=0) out<<" around "<<peak_pos;
    for (auto s: this->integrals)
        out<<"\n\t\t* "<<s.second;
}


WaveformIntegral* WaveformSignalRegion::GetIntegralPointer(UInt_t s) {
    if (s< integral_names.size())
        return &(integrals[integral_names.at(s)]);
    return 0;
}

WaveformIntegral* WaveformSignalRegion::GetIntegralPointer(std::string s) {
    if (integrals.count(s)>0)
        return &(integrals[s]);
    return 0;
}

void WaveformSignalRegion::SetPeakPostion(Int_t peak_position) {
//    std::cout<<name<<": Setting peakPosition: "<<peak_pos<<std::endl;
    peak_pos = peak_position;
}

void WaveformSignalRegion::AddIntegral(WaveformIntegral integralDef) {
//    std::cout<<"ADD Integral "<<integralDef<<std::endl;
    int count = 0;
    std::string name = integralDef.GetName();
    while (integrals.count(integralDef.GetName())){
        count++;
        std::string new_name = name+std::to_string(count);
        std::cout<<new_name<<std::endl;
        integralDef.SetName(new_name);
    }
    name = integralDef.GetName();
    this->integrals[name] = integralDef;
    integral_names.push_back(name);
//    std::cout<<"WaveformSignalRegion \'"<<this->name<<"' added Integral: "<<integralDef<<std::endl;
}

