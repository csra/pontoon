/********************************************************************
**                                                                 **
** File   : src/io/rst/ListenerCVImage.h                               **
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
#include <opencv2/core/types_c.h>

namespace io {
namespace rst {

class ListenerCVImage : public utils::Subject<boost::shared_ptr<IplImage>>{
public:
  typedef boost::shared_ptr<IplImage> ImagePtr;

  typedef std::shared_ptr<ListenerCVImage> Ptr;
  typedef IplImage DataType;
  typedef boost::shared_ptr<IplImage> DataPtr;

  ListenerCVImage(const std::string& scope);

  ~ListenerCVImage();

private:
  rsb::ListenerPtr m_Listener;
  rsb::HandlerPtr m_Handler;

  void handle(rsb::EventPtr data);
};

} // namespace rst
} // namespace io
