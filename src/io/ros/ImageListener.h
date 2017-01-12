/********************************************************************
**                                                                 **
** File   : src/io/ros/ImageListener.h                             **
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

#include <atomic>
#include <deque>
#include <memory>

#include <utils/Subject.h>

#include <image_transport/image_transport.h>
#include <ros/ros.h>

namespace pontoon {
namespace io {
namespace ros {

class ImageListener : public utils::Subject<sensor_msgs::ImageConstPtr> {
public:
  typedef std::shared_ptr<ImageListener> Ptr;
  typedef Subject::DataType DataType;

  ImageListener(const std::string &topic);

  virtual ~ImageListener();

private:
  std::shared_ptr<::ros::NodeHandle> node;
  std::shared_ptr<::image_transport::ImageTransport> image_transport;
  std::shared_ptr<::ros::AsyncSpinner> spinner;
  ::image_transport::Subscriber image_subscriber;
};

} // namespace ros
} // namespace io
} // namespace pontoon
