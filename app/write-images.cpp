/********************************************************************
**                                                                 **
** File   : app/write-images.cpp                                   **
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

#include <boost/program_options.hpp>

#include <io/ImageIO.h>
#include <io/rst/ListenerCVImage.h>

typedef pontoon::io::rst::CombinedCVImageListener ImageListener;

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description =
      "This application listens for images and saves them into files.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message")

      ("input-uri,i",
       boost::program_options::value<std::string>()->default_value(
           "/video/compressed"),
       "The input rsb uri to receive rst::vision::Image or "
       "rstexperimental::vision::EncodedImage.")

          ("encoding,e",
           boost::program_options::value<std::string>()->default_value("jpg"),
           "The output encoding to use. Can be on of ( bmp | ppm | png | jpg | "
           "jp2 | tiff ).")

              ("prefix,p",
               boost::program_options::value<std::string>()->default_value(
                   "./img_"),
               "The output file prefix")

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

  const std::string in_scope = program_options["input-uri"].as<std::string>();
  const std::string encoding = program_options["encoding"].as<std::string>();
  const std::string prefix = program_options["prefix"].as<std::string>();

  // init components
  std::mutex mutex;

  auto in = std::make_shared<ImageListener>(in_scope);
  pontoon::io::ImageIO::FileNameGenerator fg(prefix,
                                             std::string(".") + encoding);

  auto connection =
      in->connect([&mutex, &in, &fg](ImageListener::DataType image) {
        std::cout << "Image received" << std::endl;
        std::lock_guard<std::mutex> l(mutex);
        pontoon::io::ImageIO::writeImage(fg.nextFreeFilename(), image.data());
      });

  std::cerr << "Ready..." << std::endl;

  // deadlock
  std::mutex lock;
  lock.lock();
  lock.lock();
}
