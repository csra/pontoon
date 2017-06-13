/********************************************************************
**                                                                 **
** File   : app/show-images.cpp                                    **
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

#include "io/ImageIO.h"
#include "io/rst/ListenerCVImage.h"
#include "utils/SynchronizedQueue.h"
#include <boost/program_options.hpp>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef pontoon::io::rst::CombinedCVImageListener ImageListener;
typedef pontoon::utils::SynchronizedQueue<ImageListener::DataType> ImageQueue;

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description =
      "This application listens for images and shows them.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()(
      "input-uri,i",
      boost::program_options::value<std::string>()->default_value("/video"),
      "The input rsb uri to receive rst::vision::Image or "
      "rstexperimental::vision::EncodedImage.");

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

  const std::string in_scope = program_options["input-uri"].as<std::string>();

  // init components
  auto in = std::make_shared<ImageListener>(in_scope);
  ImageQueue queue(10);

  size_t frame = 0;
  auto connection =
      in->connect([&frame, &in, &queue](ImageListener::DataType image) {
        std::cerr << "received image #" << ++frame << std::endl;
        queue.push(image);
      });

  std::string window_name("pontoon-show-images");
  cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
  int key = -1;
  cv::Mat image;
  ImageQueue::Milliseconds wait_time(100);
  while (key != 27) {
    ImageQueue::DataType img;
    if (queue.try_pop_for(img,wait_time) && img.valid()) {
      image = cv::cvarrToMat(img.data().get());
      cv::imshow(window_name, image);
    }
    key = cv::waitKey(1);
  }
  std::cout << "ESCAPE received. Leaving application." << std::endl;
}
