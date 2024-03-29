/********************************************************************
**                                                                 **
** File   : src/convert/ConvertRstImageOpenCV.cpp                  **
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

#include "convert/ConvertRstImageOpenCV.h"
#include "utils/CvHelpers.h"
#include "utils/Exception.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <rst/converters/opencv/IplImageConverter.h>

using pontoon::convert::ImageEncoding;
using pontoon::convert::EncodeRstVisionImage;
using pontoon::convert::DecodeRstVisionEncodedImage;

EncodeRstVisionImage::EncodeRstVisionImage(const ImageEncoding::Type &type)
    : _Encoding(type),
      _TypeString(std::string(".") + ImageEncoding::typeToString(type)) {}

ImageEncoding::CodedPtr
EncodeRstVisionImage::encode(const boost::shared_ptr<cv::Mat> image) {
  try {
    auto time = boost::get_system_time();
    std::vector<unsigned char> result;
    int bmpsize = image->total() * 3;
    ImageEncoding::CodedPtr resultImg(
        rst::vision::EncodedImage::default_instance().New());
    resultImg->set_encoding((rst::vision::EncodedImage_Encoding)_Encoding);
    cv::imencode(_TypeString, *image, result);
    resultImg->set_data(result.data(), result.size());
    std::cerr << _TypeString << " c.f.: " << std::setprecision(4) << std::fixed
              << result.size() / (double)bmpsize << " ( in "
              << (boost::get_system_time() - time).total_nanoseconds() /
                     1000000.
              << "ms)" << std::endl;
    return resultImg;
  } catch (std::exception &e) {
    std::stringstream error;
    error << "Cannot convert: " << image.get() << " to " << _TypeString << " - "
          << e.what();
    throw utils::Exception(error.str());
  }
}

ImageEncoding::UncodedPtr
DecodeRstVisionEncodedImage::decode(const ImageEncoding::CodedPtr image) {
  return decode(*image);
}

ImageEncoding::UncodedPtr
DecodeRstVisionEncodedImage::decode(const rst::vision::EncodedImage &image) {
  try {
    auto time = boost::get_system_time();
    std::vector<unsigned char> tmp;
    tmp.resize(image.data().size());
    std::copy(image.data().begin(), image.data().end(), tmp.begin());
    boost::shared_ptr<cv::Mat> mat(new cv::Mat());
    cv::imdecode(tmp, cv::IMREAD_UNCHANGED, mat.get());
    std::cerr << ImageEncoding::typeToString(
                     (ImageEncoding::Type)image.encoding())
              << " i.f.: " << std::setprecision(4) << std::fixed
              << mat->total() * 3 / (double)image.data().size() << " ( in "
              << (boost::get_system_time() - time).total_nanoseconds() /
                     1000000.
              << "ms)" << std::endl;
    return mat;
  } catch (std::exception &e) {
    std::stringstream error;
    error << "Cannot decode image with encoding: " << image.encoding() << "  - "
          << e.what();
    throw utils::Exception(error.str());
  }
}

std::string ImageEncoding::typeToString(ImageEncoding::Type t) {
  switch (t) {
  case Type::ppm:
    return "ppm";
  case Type::png:
    return "png";
  case Type::jpg:
    return "jpg";
  case Type::jp2:
    return "jp2";
  case Type::tiff:
    return "tiff";
  default:
    std::stringstream error;
    error << "Unknown ConvertRstImageOpenCV::Type (" << t << ").";
    throw utils::Exception(error.str());
  }
}

ImageEncoding::Type ImageEncoding::stringToType(const std::string &type) {
  if (type == "ppm")
    return Type::ppm;
  if (type == "png")
    return Type::png;
  if (type == "jpg")
    return Type::jpg;
  if (type == "jp2")
    return Type::jp2;
  if (type == "tiff")
    return Type::tiff;
  throw utils::Exception(std::string("Unknown ConvertRstImageOpenCV::Type: ") +
                         type);
}
