/********************************************************************
**                                                                 **
** File   : src/io/rst/InformerCVImage.cpp                         **
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

#include "InformerCVImage.h"
#include "Informer.h"
#include "convert/ConvertRstImageOpenCV.h"
#include "convert/ScaleImageOpenCV.h"
#include "utils/Exception.h"
#include <rst/vision/EncodedImage.pb.h>
#include <rst/vision/EncodedImageCollection.pb.h>
#include <rst/vision/Images.pb.h>

using pontoon::io::rst::EncodingImageInformer;
using pontoon::io::rst::EncodingMultiImageInformer;

EncodingImageInformer::EncodingImageInformer(const std::string &uri,
                                             const std::string &encoding,
                                             double scale_width,
                                             double scale_height) {
  auto scale = std::make_shared<pontoon::convert::ScaleImageOpenCV>(
      scale_width, scale_height);
  if (encoding == "none") {
    auto out = std::make_shared<InformerCVImage>(uri);
    _callback = [scale, out](DataPtr image, const Causes &causes) {
      out->publish(scale->scale(image), causes);
    };
  } else {
    const auto encoder =
        pontoon::convert::ImageEncoding::stringToType(encoding);

    auto out = std::make_shared<Informer<::rst::vision::EncodedImage>>(uri);
    auto compress =
        std::make_shared<pontoon::convert::EncodeRstVisionImage>(encoder);
    _callback = [scale, compress, out](DataPtr image, const Causes &causes) {
      out->publish(compress->encode(scale->scale(image)), causes);
    };
  }
}

void EncodingImageInformer::publish(EncodingImageInformer::DataPtr data,
                                    const pontoon::io::Causes &causes) {
  _callback(data, causes);
}

EncodingMultiImageInformer::EncodingMultiImageInformer(
