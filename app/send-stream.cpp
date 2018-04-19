/********************************************************************
**                                                                 **
** File   : app/send-stream.cpp                                    **
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

#include "io/rst/Informer.h"
#include "io/rst/InformerCVImage.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <mutex>
#include <opencv2/videoio.hpp>

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description = "This application from an OpenCV Video source "
                            "and publishes the frames via rsb.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()("input,i", boost::program_options::value<std::string>(),
                     "The input configuration.");

  desc.add_options()(
      "output-uri,o",
      boost::program_options::value<std::string>()->default_value("/image"),
      "The output rsb uri to send the image to.");
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

  const std::string out_uri = program_options["output-uri"].as<std::string>();
  const std::string input = program_options["input"].as<std::string>();

  // send image
  cv::VideoCapture capture(input);
  if (!capture.isOpened()) {
    std::cerr << "Could not open capture device from '" << input << "'"
              << std::endl;
    return 2;
  }
  pontoon::io::rst::InformerCVImage informer(out_uri);
  auto start = cv::getTickCount();
  size_t frame_number = 0;
  size_t batch_size = 100;
  while (capture.isOpened()) {
    auto image = boost::make_shared<cv::Mat>();
    if (capture.read(*image)) {
      informer.publish(image, pontoon::io::Causes());
      if ((++frame_number % batch_size) == 0) {
        std::cerr << "\tframe: #" << frame_number << "\tfps: "
                  << cv::getTickFrequency() / (cv::getTickCount() - start) *
                         (double)batch_size
                  << std::endl;
        start = cv::getTickCount();
      }
    }
  }
  std::cerr << "Video capture closed." << std::endl;
}
