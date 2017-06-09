/********************************************************************
**                                                                 **
** File   : src/convert/CompressRstImageZlib.h                     **
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
#include <rst/vision/Image.pb.h>
#include <vector>

namespace pontoon {
namespace convert {

class CompressRstImageZlib {
public:
  typedef boost::shared_ptr<rst::vision::Image> UncompressedImagePtr;
  typedef boost::shared_ptr<rst::vision::Image> CompressedImagePtr;

  CompressedImagePtr compress(const UncompressedImagePtr);
  UncompressedImagePtr decompress(const CompressedImagePtr);

private:
  ulong m_BufferSize = 0;
  std::unique_ptr<unsigned char> m_Buffer;
};

} // namespace extract
} // namespace pontoon
