#include "eudaq/TUEvent.hh"

#include <ostream>

namespace eudaq {

  EUDAQ_DEFINE_EVENT(TUEvent, str2id("_TU"));

  TUEvent::TUEvent(Deserializer & ds) :
    Event(ds)
  {
    ds.read(m_extratimes);
  }

  void TUEvent::Print(std::ostream & os) const {
    Event::Print(os);
    if (m_extratimes.size() > 0) {
      os << " [" << m_extratimes.size() << " extra]";
    }
  }

  void TUEvent::Serialize(Serializer & ser) const {
    Event::Serialize(ser);
    ser.write(m_extratimes);
  }
}
