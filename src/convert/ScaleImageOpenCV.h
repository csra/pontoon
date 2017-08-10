/********************************************************************
**                                                                 **
** File   : src/convert/ScaleImageOpenCV.h                         **
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
#include <opencv2/core/core_c.h>

namespace pontoon {
namespace convert {

class ScaleImageOpenCV {
public:
  using ImgType = boost::shared_ptr<cv::Mat>;

  ScaleImageOpenCV(double scale_width, double scale_height);

  ImgType scale(const ImgType image) const;

private:
  double _width;
  double _height;
};

} // namespace extract
} // namespace pontoon
