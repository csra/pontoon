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

#include <mutex>
#include <memory>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>

#include <io/rst/Informer.h>
#include <io/ros/ImageListener.h>
#include <convert/ConvertRstRosImage.h>
#include <utils/SynchronizedQueue.h>

int main(int argc, char **argv){
  boost::program_options::variables_map program_options;

  std::string description = "";
  std::stringstream description_text;
  description_text << description << "\n\n" << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()
      ("help,h","produce help message")

      ("input-topic,i",
       boost::program_options::value<std::string>()->default_value("/topic"),
       "The ros topic to listen to for images.")

      ("output-scope,o",
       boost::program_options::value<std::string>()->default_value("/scope"),
       "The rsb scope to publish images to.")

      ;

  try {
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), program_options);
    boost::program_options::notify(program_options);

    std::stringstream arguments;
    for(int i = 0; i < argc; ++i){
      arguments << argv[i] << " ";
    }
    std::cout << "Program started with line: " << arguments.str();

    if (program_options.count("help")) {
      std::cout << desc << "\n";
      return 1;
    }

  } catch (boost::program_options::error& e) {
    std::stringstream arguments;
    for(int i = 0; i < argc; ++i){
      arguments << argv[i] << " ";
    }
    std::cout << "Could not parse program options: " << e.what();
    std::cout << "\n\n" << desc << "\n";
    return 1;
  }

  const std::string input = program_options["input-topic"].as<std::string>();
  const std::string output = program_options["output-scope"].as<std::string>();

  auto rosImageSource = std::make_shared<io::ros::ImageListener>(input);
  auto rsbInformer = std::make_shared<io::rst::Informer<rst::vision::Image>>(output);
  utils::SynchronizedQueue<convert::ConvertRstRosImage::RosType> queue(15);

  auto connection1 = rosImageSource->connect(
        [&rosImageSource, &queue] (io::ros::ImageListener::DataType msg) {queue.push(msg);}
  );

  for(;;){
    convert::ConvertRstRosImage::RosType ros_img;
    queue.pop(ros_img);
    auto msg = convert::ConvertRstRosImage::convert(ros_img);
    rsbInformer->publish(msg);
  }
}
