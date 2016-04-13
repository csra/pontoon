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

#include <io/ImageIO.h>
#include <utils/Exception.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/filesystem.hpp>
#include <iomanip>
#include <opencv2/highgui/highgui.hpp>

using pontoon::io::ImageIO;

ImageIO::FileNameGenerator::FileNameGenerator(
    const std::string& prefix, const std::string& suffix, int start, int padding)
  : m_Prefix(prefix), m_Suffix(suffix), m_Padding(padding),  m_Current(start) {}

std::string ImageIO::FileNameGenerator::nextFilename() {
  std::stringstream s;
  s << m_Prefix << std::setfill('0') << std::setw(m_Padding) << m_Current++ << m_Suffix;
  return s.str();
}

std::string ImageIO::FileNameGenerator::nextFreeFilename(){
  std::string name;
  do {
    name = nextFilename();
  } while (boost::filesystem::exists(name));
  return name;
}

bool ImageIO::writeImage(const std::string& file_name, boost::shared_ptr<IplImage> image){
  std::cerr << "writing file: " << file_name << std::endl;
  cvSaveImage(file_name.c_str(),image.get());
  return true;
}

boost::shared_ptr<IplImage> ImageIO::readImage(const std::string& file_name){
  return boost::shared_ptr<IplImage>(cvLoadImage(file_name.c_str()));
}
