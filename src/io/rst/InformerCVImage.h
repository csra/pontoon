/********************************************************************
**                                                                 **
** File   : src/io/rst/InformerCVImage.h                           **
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
#include <opencv2/core/core_c.h>

namespace pontoon {
namespace io {
namespace rst {

class InformerCVImage {
public:

  typedef std::shared_ptr<InformerCVImage> Ptr;
  typedef IplImage DataType;
  typedef boost::shared_ptr<DataType> DataPtr;

  InformerCVImage(const std::string& uri){
    m_Informer = utils::rsbhelpers::createInformer<DataType>(uri);
  }

  virtual ~InformerCVImage(){
  }

  virtual void publish(DataPtr data){
    m_Informer->publish(data);
  }

private:
  typename rsb::Informer<DataType>::Ptr m_Informer;
};

} // namespace rst
} // namespace io
} // namespace pontoon
