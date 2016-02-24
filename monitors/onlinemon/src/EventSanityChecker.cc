/*
 * EventSanityChecker.cc
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#include "EventSanityChecker.hh"

//default constructor
EventSanityChecker::EventSanityChecker()
{
  NPlanes=0;
  NWaveforms = 0;
}

//set the number of planes and frames
EventSanityChecker::EventSanityChecker(int nplanes, int nwaveforms)
{
  NPlanes=nplanes;
  NWaveforms=nwaveforms;
}

EventSanityChecker::~EventSanityChecker()
{
  // TODO Auto-generated destructor stub
}


unsigned int EventSanityChecker::getNPlanes() const
{
  return NPlanes;
}



void EventSanityChecker::setNPlanes(int NPlanes)
{
  this->NPlanes = NPlanes;
}



unsigned int EventSanityChecker::getNWaveforms() const
{
  return NWaveforms;
}



void EventSanityChecker::setNWaveforms(int NWaveforms)
{
  this->NWaveforms = NWaveforms;
}


