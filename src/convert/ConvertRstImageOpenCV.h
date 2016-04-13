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

#include <memory>
#include <vector>
#include <mutex>

#include <opencv2/core/types_c.h>
#include <rstexperimental/vision/EncodedImage.pb.h>

#include <utils/Subject.h>

namespace pontoon {
namespace convert {

  class ConvertRstImageOpenCV {
  public:
    typedef boost::shared_ptr<IplImage> UncodedPtr;
    typedef boost::shared_ptr<rstexperimental::vision::EncodedImage> CodedPtr;

    enum Type {
      bmp = rstexperimental::vision::EncodedImage_Encoding_BMP,
      ppm = rstexperimental::vision::EncodedImage_Encoding_PPM,
      png = rstexperimental::vision::EncodedImage_Encoding_PNG,
      jpg = rstexperimental::vision::EncodedImage_Encoding_JPG,
      jp2 = rstexperimental::vision::EncodedImage_Encoding_JP2,
      tiff = rstexperimental::vision::EncodedImage_Encoding_TIFF,
    };

    ConvertRstImageOpenCV(const Type& type);

    CodedPtr encode(const UncodedPtr);

    UncodedPtr decode(const CodedPtr);

    static std::string typeToString(Type t);
    static Type stringToType(const std::string& type);

  private:
    const Type m_Encoding;
    const std::string m_TypeString;

  };

} // namespace extract
} // namespace pontoon
