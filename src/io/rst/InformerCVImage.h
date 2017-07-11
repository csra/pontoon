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

#include "io/Cause.h"
#include "utils/RsbHelpers.h"
#include "utils/Subject.h"
#include <boost/make_shared.hpp>
#include <opencv2/core/core_c.h>
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Listener.h>
#include <rsc/runtime/TypeStringTools.h>

namespace pontoon {
namespace io {
namespace rst {

class InformerCVImage {
public:
  typedef std::shared_ptr<InformerCVImage> Ptr;
  typedef IplImage DataType;
  typedef boost::shared_ptr<DataType> DataPtr;

  InformerCVImage(const std::string &uri) {
    _Informer = utils::rsbhelpers::createInformer<DataType>(uri);
  }

  virtual ~InformerCVImage() {}

  virtual void publish(DataPtr data, const pontoon::io::Causes &causes) {
    auto event = _Informer->createEvent();
    for (auto cause : causes) {
      event->addCause(cause);
    }
    event->setData(data);
    _Informer->publish(event);
  }

private:
  typename rsb::Informer<DataType>::Ptr _Informer;
};

class EncodingImageInformer {
public:
  typedef std::shared_ptr<EncodingImageInformer> Ptr;
  typedef IplImage DataType;
  typedef boost::shared_ptr<DataType> DataPtr;

  EncodingImageInformer(const std::string &uri,
                        const std::string &encoding = "none",
                        double scale_width = 1., double scale_height = 1.);

  virtual ~EncodingImageInformer() {}

  virtual void publish(DataPtr data, const pontoon::io::Causes &causes);

private:
  std::function<void(DataPtr, pontoon::io::Causes)> _callback;
};

class EncodingMultiImageInformer {
public:
  typedef std::shared_ptr<EncodingMultiImageInformer> Ptr;
  typedef std::vector<boost::shared_ptr<IplImage>> Data;

  EncodingMultiImageInformer(const std::string &uri,
                             const std::string &encoding = "none",
                             double scale_width = 1., double scale_height = 1.);

  virtual ~EncodingMultiImageInformer() {}

  virtual void publish(Data data, const pontoon::io::Causes &causes);

private:
  std::function<void(Data, pontoon::io::Causes)> _callback;
};

} // namespace rst
} // namespace io
} // namespace pontoon
