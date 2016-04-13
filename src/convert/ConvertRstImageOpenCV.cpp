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

using pontoon::convert::ImageEncoding;
using pontoon::convert::EncodeRstVisionImage;
using pontoon::convert::DecodeRstVisionEncodedImage;

EncodeRstVisionImage::EncodeRstVisionImage(const ImageEncoding::Type& type)
  : m_Encoding(type), m_TypeString(std::string(".") + ImageEncoding::typeToString(type))
{}

ImageEncoding::CodedPtr EncodeRstVisionImage::encode(const boost::shared_ptr<IplImage> image) {
  try {
    auto time = boost::get_system_time();
    cv::Mat mat(image.get(),false);
    std::vector<unsigned char> result;
    int bmpsize = mat.total() * 3;
    ImageEncoding::CodedPtr resultImg(rstexperimental::vision::EncodedImage::default_instance().New());
    resultImg->set_encoding((rstexperimental::vision::EncodedImage_Encoding) m_Encoding);
    cv::imencode(m_TypeString,mat,result);
    resultImg->set_data(result.data(),result.size());
    std::cerr << m_TypeString << " c.f.: " << std::setprecision(4) << std::fixed
              << result.size() / (double) bmpsize << " ( in "
              << (boost::get_system_time() - time).total_nanoseconds() / 1000000. << "ms)" << std::endl;
    return resultImg;
  } catch (std::exception& e){
    std::stringstream error;
    error << "Cannot convert: " << image.get() << " to " << m_TypeString << " - " << e.what();
    throw utils::Exception(error.str());
  }
}

class CustomDeleter {
private:
  boost::shared_ptr<cv::Mat> mat;
public:

  CustomDeleter(boost::shared_ptr<cv::Mat> impl) : mat(impl) {}

  void operator()(IplImage* img) {
    delete img;
  }
};

ImageEncoding::UncodedPtr DecodeRstVisionEncodedImage::decode(const ImageEncoding::CodedPtr image) {
  try {
    auto time = boost::get_system_time();
    std::vector<unsigned char> tmp; tmp.resize(image->data().size());
    std::copy(image->data().begin(), image->data().end(), tmp.begin());
    boost::shared_ptr<cv::Mat> mat(new cv::Mat());
    cv::imdecode(tmp,cv::IMREAD_UNCHANGED,mat.get());
    std::cerr << ImageEncoding::typeToString((ImageEncoding::Type) image->encoding()) << " i.f.: "
              << std::setprecision(4) << std::fixed
              << mat->total() * 3 / (double) image->data().size() << " ( in "
              << (boost::get_system_time() - time).total_nanoseconds() / 1000000. << "ms)" << std::endl;
    return ImageEncoding::UncodedPtr(new IplImage(*mat),CustomDeleter(mat));
  } catch (std::exception& e){
    std::stringstream error;
    error << "Cannot decode image with encoding: " << image->encoding() << "  - " << e.what();
    throw utils::Exception(error.str());
  }
}

std::string ImageEncoding::typeToString(ImageEncoding::Type t){
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

ImageEncoding::Type ImageEncoding::stringToType(const std::string& type){
  if(type == "bmp" ) return Type::bmp;
  if(type == "ppm" ) return Type::ppm;
  if(type == "png" ) return Type::png;
  if(type == "jpg" ) return Type::jpg;
  if(type == "jp2" ) return Type::jp2;
  if(type == "tiff") return Type::tiff;
  throw utils::Exception(std::string("Unknown ConvertRstImageOpenCV::Type: ") + type);
}
