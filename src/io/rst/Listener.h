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

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Listener.h>
#include <utils/RsbHelpers.h>
#include <utils/Subject.h>
#include <rsc/runtime/TypeStringTools.h>
#include <boost/make_shared.hpp>

namespace pontoon {
namespace io {
namespace rst {

template<typename RST>
class Listener : public utils::Subject<boost::shared_ptr<RST>>{
public:

  typedef std::shared_ptr<Listener<RST>> Ptr;
  typedef RST DataType;
  typedef boost::shared_ptr<RST> DataPtr;

  Listener(const std::string& url)
    : m_Type(rsc::runtime::typeName(typeid(RST)))
  {
    utils::rsbhelpers::register_rst<RST>();
    m_Listener = utils::rsbhelpers::createListener(url);
    m_Handler = boost::make_shared<rsb::EventFunctionHandler>(boost::bind(&Listener<RST>::handle,this,_1));
    m_Listener->addHandler(m_Handler);
  }

  virtual ~Listener(){
    m_Listener->removeHandler(m_Handler);
  }

  void handle(rsb::EventPtr event){
    if(event->getType() == m_Type){
      this->notify(boost::static_pointer_cast<RST>(event->getData()));
    }
  }

private:
  const std::string m_Type;
  rsb::ListenerPtr m_Listener;
  rsb::HandlerPtr m_Handler;
};

} // namespace rst
} // namespace io
} // namespace pontoon
