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

#include <memory>
#include <vector>
#include <mutex>

#include <opencv2/core/types_c.h>
#include <boost/shared_ptr.hpp>

namespace pontoon {
namespace io {

  class ImageIO {
  public:

    class FileNameGenerator {
    public:
      FileNameGenerator(const std::string& prefix, const std::string& suffix, int start = 0, int padding = 6);

      std::string nextFilename();
      std::string nextFreeFilename();
    private:
      std::string m_Prefix;
      std::string m_Suffix;
      int m_Padding;
      int m_Current;
    };

    static bool writeImage(const std::string& file_name, boost::shared_ptr<IplImage> image);
    static boost::shared_ptr<IplImage> readImage(const std::string& file_name);

  };

} // namespace io
} // namespace pontoon
