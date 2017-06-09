/********************************************************************
**                                                                 **
** File   : src/convert/ConvertRstImageOpenCV.h                    **
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

#include "utils/Subject.h"
#include <memory>
#include <mutex>
#include <opencv2/core/core_c.h>
#include <rst/vision/EncodedImage.pb.h>
#include <vector>

namespace pontoon {
namespace convert {

struct ImageEncoding {

  typedef boost::shared_ptr<IplImage> UncodedPtr;
  typedef boost::shared_ptr<rst::vision::EncodedImage> CodedPtr;

  enum Type {
    ppm = rst::vision::EncodedImage_Encoding_PPM,
    png = rst::vision::EncodedImage_Encoding_PNG,
    jpg = rst::vision::EncodedImage_Encoding_JPG,
    jp2 = rst::vision::EncodedImage_Encoding_JP2,
    tiff = rst::vision::EncodedImage_Encoding_TIFF,
  };

  static std::string typeToString(Type t);
  static Type stringToType(const std::string &type);
};

class EncodeRstVisionImage {
public:
  EncodeRstVisionImage(const ImageEncoding::Type &type);

  ImageEncoding::CodedPtr encode(const ImageEncoding::UncodedPtr);

private:
  const ImageEncoding::Type m_Encoding;
  const std::string m_TypeString;
};

class DecodeRstVisionEncodedImage {
public:
  ImageEncoding::UncodedPtr decode(const ImageEncoding::CodedPtr);
};

} // namespace extract
} // namespace pontoon
