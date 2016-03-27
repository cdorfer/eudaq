/*
 * WaveformSignalRegion.h
 *
 *  Created on: Oct 13, 2015
 *      Author: bachmair
 */

#ifndef WAVEFORMSIGNALREGION_H_
#define WAVEFORMSIGNALREGION_H_

#include "WaveformIntegral.hh"
#include <limits>
#include <vector>
#include <map>
#include "TROOT.h"
class WaveformSignalRegion:public TObject {
    public:
        WaveformSignalRegion(int low=-1, int high=-1, std::string name="");
        virtual ~WaveformSignalRegion();
        void SetPeakPostion(Int_t peak_position);
        UInt_t GetNIntegrals(){return integrals.size();}
        WaveformIntegral* GetIntegralPointer(UInt_t i);
        WaveformIntegral* GetIntegralPointer(std::string s);
        void ResetIntegrals();
        void Print() const {Print(std::cout);}
        void Print(std::ostream& out) const;
        void SetPolarity(signed char pol){polarity=pol;}
        void SetPulserPolarity(signed char pol){pulserPolarity=pol;}
        void AddIntegral(WaveformIntegral integralDef);
        int GetLowBoarder(){return low_border;}
        int GetHighBoarder(){return high_border;}
        int GetPeakPosition(){return peak_pos;}
        void SetName(std::string name){this->name = name;}
        const char *GetName(){return name.c_str();}
        float operator [](int i){return integrals[integral_names[i]].GetIntegral();}
        float operator [](std::string s){return integrals[s].GetIntegral();}
    private:
        std::string name;
        signed char polarity;
        signed char pulserPolarity;
        int low_border;
        int high_border;
        int peak_pos;
        std::map<std::string,WaveformIntegral> integrals;
        std::vector<std::string> integral_names;
//        ClassDef(WaveformSignalRegion,1);
};

inline std::ostream & operator << (std::ostream & os, const WaveformSignalRegion & c){c.Print(os);return os;}
#endif /* WAVEFORMSIGNALREGION_H_ */
