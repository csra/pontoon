/********************************************************************
**                                                                 **
** File   : app/image                                              **
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

#include "convert/ConvertRstRosImage.h"
#include "io/ros/ImageListener.h"
#include "io/rst/Informer.h"
#include "utils/SynchronizedQueue.h"
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <mutex>

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description = "";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()(
      "input-topic,i",
      boost::program_options::value<std::string>()->default_value("/topic"),
      "The ros topic to listen to for images.");

  desc.add_options()(
      "output-uri,o",
      boost::program_options::value<std::string>()->default_value("/scope"),
      "The rsb uri to publish images to.");

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
    std::cout << "Program started with line: " << arguments.str();

    if (program_options.count("help")) {
      std::cout << desc << "\n";
      return 1;
    }

  } catch (boost::program_options::error &e) {
    std::stringstream arguments;
    for (int i = 0; i < argc; ++i) {
      arguments << argv[i] << " ";
    }
    std::cout << "Could not parse program options: " << e.what();
    std::cout << "\n\n" << desc << "\n";
    return 1;
  }

  const std::string input = program_options["input-topic"].as<std::string>();
  const std::string output = program_options["output-uri"].as<std::string>();

  auto rosImageSource =
      std::make_shared<pontoon::io::ros::ImageListener>(input);
  auto rsbInformer =
      std::make_shared<pontoon::io::rst::Informer<rst::vision::Image>>(output);
  pontoon::utils::SynchronizedQueue<
      pontoon::convert::ConvertRstRosImage::RosType>
      queue(15);

  auto connection1 = rosImageSource->connect([&rosImageSource, &queue](
      pontoon::io::ros::ImageListener::DataType msg) { queue.push(msg); });

  for (;;) {
    pontoon::convert::ConvertRstRosImage::RosType ros_img;
    queue.pop(ros_img);
    auto msg = pontoon::convert::ConvertRstRosImage::convert(ros_img);
    rsbInformer->publish(msg, pontoon::io::Causes());
  }
}
