/********************************************************************
**                                                                 **
** File   : src/io/rst/Listener.h                               **
** Authors: Viktor Richter                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
********************************************************************/

#pragma once

#include "io/Cause.h"
#include "utils/RsbHelpers.h"
#include "utils/Subject.h"
#include <boost/make_shared.hpp>
#include <rsb/Event.h>
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Listener.h>
#include <rsb/filter/ScopeFilter.h>
#include <rsb/filter/TypeFilter.h>
#include <rsc/runtime/TypeStringTools.h>

namespace pontoon {
namespace io {
namespace rst {

class EventDataBase {
public:
  using EventPtr = boost::shared_ptr<rsb::Event>;
  using Cause = ::pontoon::io::Cause;

  EventDataBase() {}
  EventDataBase(EventPtr event) : _event(event) {}
  virtual ~EventDataBase() = default;

  virtual EventPtr event() const final { return _event; }
  virtual bool valid() const { return _event.get() != nullptr; }
  virtual Cause id() const { return _event->getId(); }
  virtual Causes causes() const { return _event->getCauses(); }

  explicit operator bool() const { return this->valid(); }

private:
  EventPtr _event;
};

template <typename RST> class EventData : public EventDataBase {
public:
  typedef RST DataType;
  typedef boost::shared_ptr<DataType> DataPtr;

  EventData() : EventDataBase() {}
  EventData(EventPtr event) : EventDataBase(event) {}

  DataPtr data() const {
    return boost::static_pointer_cast<RST>(event()->getData());
  }

  bool valid() const {
    return EventDataBase::valid() &&
           event()->getType() == rsc::runtime::typeName<RST>();
  }
};

template <typename RST> class EventDataVector : public EventDataBase {
public:
  using DataType = std::vector<boost::shared_ptr<RST>>;

  EventDataVector() {}
  EventDataVector(EventPtr event, DataType data)
      : EventDataBase(event), _data(data) {}

  virtual ~EventDataVector() = default;

  const DataType &data() const { return _data; }

private:
  EventPtr _event;
  DataType _data;
};

template <typename RST> class Listener : public utils::Subject<EventData<RST>> {
public:
  typedef std::shared_ptr<Listener<RST>> Ptr;

  Listener(const std::string &uri, bool filter_subscopes = false)
      : _Type(rsc::runtime::typeName(typeid(RST))) {
    utils::rsbhelpers::register_rst<RST>();
    _Listener = utils::rsbhelpers::createListener(uri);
    _Listener->addFilter(
        rsb::filter::FilterPtr(rsb::filter::TypeFilter::createForType<RST>()));
    if (filter_subscopes) {
      _Listener->addFilter(rsb::filter::FilterPtr(new rsb::filter::ScopeFilter(
          rsb::Scope(pontoon::utils::rsbhelpers::parseScope(uri)))));
    }
    _Handler = boost::make_shared<rsb::EventFunctionHandler>(
        boost::bind(&Listener<RST>::handle, this, _1));
    _Listener->addHandler(_Handler);
  }

  virtual ~Listener() { _Listener->removeHandler(_Handler); }

  void handle(rsb::EventPtr event) { this->notify(EventData<RST>(event)); }

  const std::string &type() const { return _Type; }

private:
  const std::string _Type;
  rsb::ListenerPtr _Listener;
  rsb::HandlerPtr _Handler;
};

} // namespace rst
} // namespace io
} // namespace pontoon
