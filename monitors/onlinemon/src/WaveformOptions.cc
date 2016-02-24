/*
 * WaveformOptions.cc
 *
 *  Created on: Apr 16, 2015
 *      Author: bachmair
 */

#include <WaveformOptions.hh>
WaveformOptions::WaveformOptions() {
    // TODO Auto-generated constructor stub
    SetDefaults();
}

WaveformOptions::~WaveformOptions() {
    // TODO Auto-generated destructor stub
}

void WaveformOptions::SetDefaults(){
    KeyValueMap["signal_range_min"] = "200";
    KeyValueMap["signal_range_max"] = "350";

}
