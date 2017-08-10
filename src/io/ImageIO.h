/********************************************************************
**                                                                 **
** File   : src/io/ImageIO.h                                       **
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

#include <boost/shared_ptr.hpp>
#include <memory>
#include <mutex>
#include <opencv2/core/core_c.h>
#include <vector>

namespace pontoon {
namespace io {

class ImageIO {
public:
  class FileNameGenerator {
  public:
    FileNameGenerator(const std::string &prefix, const std::string &suffix,
                      int start = 0, int padding = 6);

    std::string nextFilename();
    std::string nextFreeFilename();

  private:
    std::string _Prefix;
    std::string _Suffix;
    int _Padding;
    int _Current;
  };

  static bool writeImage(const std::string &file_name, const cv::Mat &image);
  static bool writeIplImage(const std::string &file_name,
                            const IplImage &image);
  static boost::shared_ptr<cv::Mat> readImage(const std::string &file_name);
  static boost::shared_ptr<IplImage> readIplImage(const std::string &file_name);
};

} // namespace io
} // namespace pontoon
