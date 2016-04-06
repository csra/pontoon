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

#include <convert/ConvertRstImageOpenCV.h>
#include <utils/Exception.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>
#include <rst/converters/opencv/IplImageConverter.h>

#include <opencv2/highgui/highgui.hpp>

using convert::ConvertRstImageOpenCV;

ConvertRstImageOpenCV::ConvertRstImageOpenCV(const Type& type)
  : m_Encoding(type), m_TypeString(std::string(".") + typeToString(type))
{}

ConvertRstImageOpenCV::CodedPtr ConvertRstImageOpenCV::encode(const boost::shared_ptr<IplImage> image) {
  try {
    cv::Mat mat(image.get(),false);
    std::vector<unsigned char> result;
    int bmpsize = mat.total() * 3;
    CodedPtr resultImg(rstexperimental::vision::EncodedImage::default_instance().New());
    resultImg->set_encoding((rstexperimental::vision::EncodedImage_Encoding) m_Encoding);
    auto time = boost::get_system_time();
    cv::imencode(m_TypeString,mat,result);
    std::cerr << m_TypeString << " c.f.: " << std::setprecision(4) << std::fixed
              << result.size() / (double) bmpsize << " ( in "
              << (boost::get_system_time() - time).total_nanoseconds() / 1000000. << "ms)" << std::endl;
    resultImg->set_data(result.data(),result.size());
    return resultImg;
  } catch (std::exception& e){
    std::stringstream error;
    error << "Cannot convert: " << image.get() << " to " << m_TypeString << " - " << e.what();
    throw utils::Exception(error.str());
  }
}


ConvertRstImageOpenCV::UncodedPtr ConvertRstImageOpenCV::decode(const ConvertRstImageOpenCV::CodedPtr image) {
  try {
    std::vector<unsigned char> tmp; tmp.resize(image->data().size());
    std::copy(image->data().begin(), image->data().end(), tmp.begin());
    cv::Mat mat = cv::imdecode(tmp,cv::IMREAD_UNCHANGED);
    return UncodedPtr(new IplImage(mat));
  } catch (std::exception& e){
    std::stringstream error;
    error << "Cannot decode image with encoding: " << image->encoding() << "  - " << e.what();
    throw utils::Exception(error.str());
  }
}

std::string ConvertRstImageOpenCV::typeToString(ConvertRstImageOpenCV::Type t){
  switch(t){
    case Type::bmp:
      return "bmp";
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

ConvertRstImageOpenCV::Type ConvertRstImageOpenCV::stringToType(const std::string& type){
  if(type == "bmp" ) return Type::bmp;
  if(type == "ppm" ) return Type::ppm;
  if(type == "png" ) return Type::png;
  if(type == "jpg" ) return Type::jpg;
  if(type == "jp2" ) return Type::jp2;
  if(type == "tiff") return Type::tiff;
  throw utils::Exception(std::string("Unknown ConvertRstImageOpenCV::Type: ") + type);
}
