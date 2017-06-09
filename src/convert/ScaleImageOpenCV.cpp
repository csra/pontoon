/********************************************************************
**                                                                 **
** File   : src/convert/ScaleImageOpenCV.cpp                       **
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

#include "convert/ScaleImageOpenCV.h"
#include "utils/CvHelpers.h"
#include "utils/Exception.h"
#include <opencv2/imgproc.hpp>

using pontoon::convert::ScaleImageOpenCV;
typedef ScaleImageOpenCV::ImgType ImgType;

ScaleImageOpenCV::ScaleImageOpenCV(double scale_width, double scale_height)
    : _width(scale_width), _height(scale_height) {}

ImgType ScaleImageOpenCV::scale(const ImgType image) const {
  cv::InterpolationFlags interpol = cv::INTER_LINEAR;
  if (_width == 1. && _height == 1.) {
    return image; // scaling not needed
  } else if (_width < 1. || _height < 1.) {
    interpol = cv::INTER_AREA;
  } else {
    interpol = cv::INTER_CUBIC;
  }
  cv::Mat src = cv::cvarrToMat(image.get());
  boost::shared_ptr<cv::Mat> dst(new cv::Mat());
  cv::resize(src, *dst, cv::Size(), _width, _height, interpol);
  return pontoon::utils::cvhelpers::asIplImagePtr(dst);
}
