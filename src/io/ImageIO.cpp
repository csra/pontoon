/********************************************************************
**                                                                 **
** File   : src/io/ImageIO.cpp                                     **
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

#include "io/ImageIO.h"
#include "utils/Exception.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread_time.hpp>
#include <iomanip>
#include <opencv2/highgui/highgui.hpp>

using pontoon::io::ImageIO;

ImageIO::FileNameGenerator::FileNameGenerator(const std::string &prefix,
                                              const std::string &suffix,
                                              int start, int padding)
    : _Prefix(prefix), _Suffix(suffix), _Padding(padding), _Current(start) {}

std::string ImageIO::FileNameGenerator::nextFilename() {
  std::stringstream s;
  s << _Prefix << std::setfill('0') << std::setw(_Padding) << _Current++
    << _Suffix;
  return s.str();
}

std::string ImageIO::FileNameGenerator::nextFreeFilename() {
  std::string name;
  do {
    name = nextFilename();
  } while (boost::filesystem::exists(name));
  return name;
}

bool ImageIO::writeImage(const std::string &file_name, const cv::Mat &image) {
  int written = cv::imwrite(file_name.c_str(), image);
  if (written) {
    std::cerr << "file written: " << file_name << std::endl;
  } else {
    std::cerr << "error writing file: " << file_name << std::endl;
  }
  return true;
}

bool ImageIO::writeIplImage(const std::string &file_name,
                            const IplImage &image) {
  int written = cvSaveImage(file_name.c_str(), &image);
  if (written) {
    std::cerr << "file written: " << file_name << std::endl;
  } else {
    std::cerr << "error writing file: " << file_name << std::endl;
  }
  return true;
}

class IplImageDeleter {
public:
  void operator()(IplImage *img) { cvReleaseImage(&img); }
};

boost::shared_ptr<cv::Mat> ImageIO::readImage(const std::string &file_name) {
  return boost::make_shared<cv::Mat>(cv::imread(file_name.c_str()));
}

boost::shared_ptr<IplImage>
ImageIO::readIplImage(const std::string &file_name) {
  return boost::shared_ptr<IplImage>(cvLoadImage(file_name.c_str()),
                                     IplImageDeleter());
}
