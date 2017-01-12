/********************************************************************
**                                                                 **
** File   : app/pacemaker.cpp                                      **
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

#include <boost/program_options.hpp>
#include <iostream>

#include <io/ros/Informer.h>
#include <io/rst/Informer.h>
#include <rst/timing/Timestamp.pb.h>
#include <std_msgs/UInt64.h>

boost::shared_ptr<rst::timing::Timestamp> create_rsb_time(uint timestamp) {
  using rst::timing::Timestamp;
  auto result = boost::make_shared<Timestamp>();
  result->set_time(timestamp);
  return result;
}

std_msgs::UInt64 create_ros_time(uint timestamp) {
  std_msgs::UInt64 result;
  result.data = timestamp;
  return result;
}

void sleep_until(uint last_time_micros, uint sum_wait_micros) {
  uint waited = rsc::misc::currentTimeMicros() - last_time_micros;
  std::cout << "...needed: " << waited << " ...waiting for "
            << sum_wait_micros - waited << std::endl;
  usleep(sum_wait_micros - waited);
}

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description =
      "This application periodically sends its current time via ros and rsb.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message")

      ("delta,d", boost::program_options::value<uint>()->default_value(100),
       "The time between two messages in milliseconds.")

          ("uri,u", boost::program_options::value<std::string>()->default_value(
                        "/pacemaker"),
           "The output rsb uri to send the notifications to.")

              ("topic,t",
               boost::program_options::value<std::string>()->default_value(
                   "/pacemaker"),
               "The output ros topic to send the notifications to.")

      ;

  try {
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc),
        program_options);
    boost::program_options::notify(program_options);

    std::stringstream arguments;
    for (int i = 0; i < argc; ++i) {
      arguments << argv[i] << " ";
    }
    std::cerr << "Program started with line: " << arguments.str() << std::endl;

    if (program_options.count("help")) {
      std::cout << desc << "\n";
      return 1;
    }

  } catch (boost::program_options::error &e) {
    std::stringstream arguments;
    for (int i = 0; i < argc; ++i) {
      arguments << argv[i] << " ";
    }
    std::cerr << "Could not parse program options: " << e.what();
    std::cerr << "\n\n" << desc << "\n";
    return 1;
  }

  const std::string uri = program_options["uri"].as<std::string>();
  const std::string topic = program_options["topic"].as<std::string>();
  const uint delta = program_options["delta"].as<uint>();

  auto rsb_informer = pontoon::io::rst::Informer<rst::timing::Timestamp>(uri);
  auto ros_informer =
      pontoon::io::ros::Informer<std_msgs::UInt64>(topic, "pacemaker");

  std::cout << "Ready. Sending the current time every " << delta
            << " milliseconds" << std::endl;

  while (true) {
    uint timestamp = rsc::misc::currentTimeMicros();
    auto rsb_time = create_rsb_time(timestamp);
    auto ros_time = create_ros_time(timestamp);
    rsb_informer.publish(rsb_time);
    ros_informer.publish(ros_time);
    std::cout << "Microseconds since utc-epoch: " << timestamp << std::endl;
    sleep_until(timestamp, delta * 1000);
  }
}
