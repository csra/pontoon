/********************************************************************
**                                                                 **
** File   : src/io/ros/ImageListener.cpp                           **
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

#include <io/ros/ImageListener.h>

using pontoon::io::ros::ImageListener;

ImageListener::ImageListener(const std::string &topic) {
  ::ros::init(std::map<std::string, std::string>(), "",
              ::ros::init_options::NoSigintHandler);
  node = std::make_shared<::ros::NodeHandle>();
  image_transport = std::make_shared<::image_transport::ImageTransport>(*node);
  spinner = std::make_shared<::ros::AsyncSpinner>(1);
  auto callback = [this](const ::sensor_msgs::ImageConstPtr msg) {
    this->notify(msg);
  };
  image_subscriber = image_transport->subscribe(topic, 1, callback);
  spinner->start();
}

ImageListener::~ImageListener() { spinner->stop(); }
