/********************************************************************
**                                                                 **
** File   : src/convert/ConvertRstRosImage.h                       **
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

#include <image_transport/image_transport.h>
#include <rst/vision/Image.pb.h>

namespace pontoon {
namespace convert {

class ConvertRstRosImage {
public:
  typedef sensor_msgs::ImageConstPtr RosType;
  typedef boost::shared_ptr<rst::vision::Image> RstType;

  static boost::shared_ptr<rst::vision::Image>
  convert(const sensor_msgs::ImageConstPtr src);
  static sensor_msgs::ImageConstPtr
  convert(boost::shared_ptr<rst::vision::Image> src);
};

} // namespace convert
} // namespace pontoon
