/********************************************************************
**                                                                 **
** File   : src/io/rst/Informer.h                               **
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
#include <rsb/Factory.h>
#include <rsb/Informer.h>
#include <rsc/runtime/TypeStringTools.h>

namespace pontoon {
namespace io {
namespace rst {

template <typename RST> class Informer {
public:
  typedef std::shared_ptr<Informer<RST>> Ptr;
  typedef RST DataType;
  typedef boost::shared_ptr<RST> DataPtr;

  Informer(const std::string &uri) {
    utils::rsbhelpers::register_rst<RST>();
    m_Informer = utils::rsbhelpers::createInformer<RST>(uri);
  }

  virtual ~Informer() {}

  virtual void publish(DataPtr data, const pontoon::io::Causes &causes) {
    auto event = m_Informer->createEvent();
    for (auto cause : causes) {
      event->addCause(cause);
    }
    m_Informer->publish(event);
  }

private:
  typename rsb::Informer<RST>::Ptr m_Informer;
};

template <typename RST> class Publisher : private Informer<RST> {
public:
  typedef std::shared_ptr<Publisher<RST>> Ptr;

  Publisher(const std::string &scope,
            typename utils::Subject<boost::shared_ptr<RST>>::Ptr subject)
      : Informer<RST>(scope) {
    m_Connection = subject->connect(
        [this](boost::shared_ptr<RST> data) { this->publish(data); });
  }

  ~Publisher() { m_Connection.disconnect(); }

private:
  typename utils::Subject<boost::shared_ptr<RST>>::Connection m_Connection;
};

} // namespace rst
} // namespace io
} // namespace pontoon
