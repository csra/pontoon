/********************************************************************
**                                                                 **
** File   : src/utils/CvHelpers.cpp                                **
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

#include "CvHelpers.h"

namespace {
class CustomMatDeleter {
private:
  boost::shared_ptr<cv::Mat> data;

public:
  CustomMatDeleter(boost::shared_ptr<cv::Mat> impl) : data(impl) {}

  void operator()(IplImage *img) { delete img; }
};
class CustomImageDeleter {
private:
  boost::shared_ptr<IplImage> data;

public:
  CustomImageDeleter(boost::shared_ptr<IplImage> impl) : data(impl) {}

  void operator()(cv::Mat *img) { delete img; }
};
}

boost::shared_ptr<IplImage>
pontoon::utils::cvhelpers::asIplImagePtr(boost::shared_ptr<cv::Mat> mat) {
  return boost::shared_ptr<IplImage>(new IplImage(*mat), CustomMatDeleter(mat));
}

boost::shared_ptr<cv::Mat>
pontoon::utils::cvhelpers::asMatPtr(boost::shared_ptr<IplImage> image) {
  return boost::shared_ptr<cv::Mat>(
      new cv::Mat(cv::cvarrToMat(image.get(), false)),
      CustomImageDeleter(image));
}
