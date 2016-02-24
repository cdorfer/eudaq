/*
 * EventSanityChecker.hh
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#ifndef EVENTSANITY_H_
#define EVENTSANITY_H_

class EventSanityChecker
{
  public:

    EventSanityChecker();
    EventSanityChecker(int nplanes, int nwaveforms);
    virtual ~EventSanityChecker();
    unsigned int getNPlanes() const;
    void setNPlanes(int NPlanes);
    unsigned int getNWaveforms() const;
    void setNWaveforms(int NWaveforms);
  private:
    unsigned int NPlanes;
    unsigned int NWaveforms;

};

#endif /* EVENTSANITY_H_ */
