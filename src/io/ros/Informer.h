/********************************************************************
**                                                                 **
** File   : src/io/ros/Informer.h                                  **
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
#include <ros/ros.h>

namespace pontoon {
namespace io {
namespace ros {

template<typename MSGType>
class Informer {
public:

  typedef std::shared_ptr<Informer<MSGType>> Ptr;
  typedef MSGType DataType;

  Informer(const std::string& topic, const std::string& name , uint queue = 0){
    ::ros::init(std::map<std::string,std::string>(), name, ::ros::init_options::NoSigintHandler);
    node = std::make_shared< ::ros::NodeHandle>();
    publisher = node->advertise<DataType>(topic,queue);
  }

  virtual ~Informer() = default;

  virtual void publish(DataType data){
    publisher.publish(data);
    ::ros::spinOnce();
  }

private:
  std::shared_ptr< ::ros::NodeHandle> node;
  ::ros::Publisher publisher;
};

} // namespace ros
} // namespace io
} // namespace pontoon
