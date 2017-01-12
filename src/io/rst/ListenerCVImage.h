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
#include <rsc/runtime/TypeStringTools.h>
#include <rst/vision/EncodedImage.pb.h>

#include <io/rst/Listener.h>
#include <utils/RsbHelpers.h>
#include <utils/Subject.h>

#include <opencv2/core/core_c.h>

#include <boost/make_shared.hpp>

namespace pontoon {
namespace io {
namespace rst {

class ListenerCVImageRstImage : public pontoon::utils::Subject<boost::shared_ptr<IplImage>>{
public:
  ListenerCVImageRstImage(const std::string& uri);

  ~ListenerCVImageRstImage();

private:
  rsb::ListenerPtr m_Listener;
  rsb::HandlerPtr m_Handler;

  void handle(rsb::EventPtr data);
};

class ListenerCVImageRstEncodedImage : public pontoon::utils::Subject<boost::shared_ptr<IplImage>>{
public:

  ListenerCVImageRstEncodedImage(const std::string& uri);

  ~ListenerCVImageRstEncodedImage();

private:
  typedef pontoon::io::rst::Listener<::rst::vision::EncodedImage> ListenerType;
  ListenerType m_Listener;
  ListenerType::Connection m_Connection;
};

class CombinedCVImageListener : public pontoon::utils::CompositeSubject<boost::shared_ptr<IplImage>>{
public:
  CombinedCVImageListener(const std::string& uri);

  ~CombinedCVImageListener() = default;
};


} // namespace rst
} // namespace io
} // namespace pontoon
