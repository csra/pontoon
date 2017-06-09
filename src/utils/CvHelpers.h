/********************************************************************
**                                                                 **
** File   : src/utils/CvHelpers.h                                  **
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
#include <opencv2/core.hpp>

namespace pontoon {
namespace utils {
namespace cvhelpers {

boost::shared_ptr<IplImage> asIplImagePtr(boost::shared_ptr<cv::Mat> mat);

} // namespace cvhelpers
} // namespace utils
} // namespace pontoon
