/*
 * SimpleStandardWaveform.cxx
 *
 *  Created on: Jun 9, 2011
 *      Author: stanitz
 */

#include "include/SimpleStandardWaveform.hh"

// marc SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name,  const int id, unsigned int nsamples, OnlineMonConfiguration* mymon ) :
SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name,  const int id, unsigned int nsamples, OnlineMonConfiguration* mymon ) :
	_name(name), _id(id), calculated(false),_nsamples(nsamples), _sign(-1),_channelnumber(-1),_PulserEvent(false)
{
}

// marc SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name, const int id, unsigned int nsamples) :
SimpleStandardWaveform::SimpleStandardWaveform(const std::string & name, const int id,  unsigned int nsamples) :
		_name(name), _id(id), calculated(false),_nsamples(nsamples),_sign(-1),_channelnumber(-1),_PulserEvent(false)
{

}

void SimpleStandardWaveform::addData(float* data) {
	this->_data=data;
}


struct myclass {
  bool operator() (int i,int j) { return i<j; }
} myobj;


void SimpleStandardWaveform::Calculate() {
	if (calculated)
		return;
	_min = *std::min_element(_data,_data+_nsamples);//,myobj);
	_max = *std::max_element(_data,_data+_nsamples);//,myobj);
	_integral = 0;
	for (int i = 0; i < _nsamples;i++)
		_integral += _data[i];
	_integral/=_nsamples;
//	std::cout<<" SimpleStandardWaveform::Calculate "<<_min<<"-"<<_max<<" "<<_integral<<std::endl;
	this->calculated = true;
}

float SimpleStandardWaveform::getIntegral(float min, float max) const {
	float integral = 0;
	int i;
	for (i = min; i <= int(max+1) && i < _nsamples;i++){
		integral += _data[i];
	}
	return integral/(float)(i-(int)min);
}

float SimpleStandardWaveform::maxSpreadInRegion(float min, float max) const {
    float minimum = getMinimum(min, max);
    float maximum = getMaximum(min, max);
    float spread = maximum-minimum;
    return spread;

}

float SimpleStandardWaveform::getAbsMaximum(float min, float max) const {
    float maxVal = -999;
    int imax = min;
    for (int i = min; i <= int(max+1) && i < _nsamples ;i++){
        if (abs(_data[i]) > maxVal){ maxVal = abs(_data[i]); imax = i; }
    }
    maxVal = _data[imax];
    return maxVal;
}

void SimpleStandardWaveform::performFFT(TVirtualFFT *fft_own) {
    float mean_fft=0; float max_fft=0; float min_fft=0;

    Double_t *re_full = new Double_t[1024];
    Double_t *im_full = new Double_t[1024];
    Double_t *in = new Double_t[1024];

    // fill the values into the input
    for (int j = 0; j < 1024; ++j) {
        in[j] = _data[j];
    }
    //set points and transform
    fft_own->SetPoints(in);
    fft_own->Transform();
    //get the transformed values
    fft_own->GetPointsComplex(re_full,im_full);

    float abs_fft;
    std::vector<float> fft_vals;
    for (int j = 0; j < 513; ++j) {
        abs_fft = TMath::Sqrt(re_full[j]*re_full[j] + im_full[j]*im_full[j]);
        fft_vals.push_back( abs_fft );
    }
    _mean_fft = std::accumulate (fft_vals.begin(), fft_vals.end(), 0.0)  / fft_vals.size();
    _max_fft = *std::max_element(fft_vals.begin(), fft_vals.end());
    _min_fft = *std::min_element(fft_vals.begin(), fft_vals.end());

    delete re_full;
    delete im_full;
    delete in;

}


float SimpleStandardWaveform::getMaximum(float min, float max) const {
    int imax = min;
    if (imax<0 || imax >= _nsamples)
        return std::numeric_limits<double>::quiet_NaN();
    float maxVal = _data[imax];
    for (int i = min+1; i <= int(max+1) && i < _nsamples ;i++){
        if ((_data[i]) > maxVal){ maxVal = (_data[i]); imax = i; }
    }
    maxVal = _data[imax];
    return maxVal;
}

float SimpleStandardWaveform::getMinimum(float min, float max) const {
    int imin = min;
    if (imin < 0 || imin >= _nsamples)
        return std::numeric_limits<double>::quiet_NaN();
    float minVal = _data[imin];
    for (int i = imin+1; i <= int(max+1) && i < _nsamples ;i++){
        if (_data[i] < minVal){ minVal = _data[i]; imin = i; }
    }
    minVal = _data[imin];
    return minVal;
}

