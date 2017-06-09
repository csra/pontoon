/********************************************************************
**                                                                 **
** File   : app/encode-images.cpp                                  **
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

#include "convert/ConvertRstImageOpenCV.h"
#include "convert/ScaleImageOpenCV.h"
#include "io/rst/Informer.h"
#include "io/rst/InformerCVImage.h"
#include "io/rst/ListenerCVImage.h"

typedef pontoon::io::rst::ListenerCVImageRstImage ImageListener;
typedef pontoon::io::rst::InformerCVImage ImageInformer;
typedef pontoon::io::rst::Informer<rst::vision::EncodedImage> EncodedImageInformer;

using pontoon::convert::ImageEncoding;
using pontoon::convert::ScaleImageOpenCV;
using pontoon::convert::EncodeRstVisionImage;


void block() {
  std::cerr << "Ready..." << std::endl;
  // deadlock
  std::mutex lock;
  lock.lock();
  lock.lock();
}

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description = "This application listens for published images and "
                            "publishes encoded versions.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()(
      "input-uri,i",
      boost::program_options::value<std::string>()->default_value("/video/raw"),
      "The input rsb uri to receive raw images.");

  desc.add_options()(
      "output-uri,o",
      boost::program_options::value<std::string>()->default_value(
          "/video/encoded"),
      "The output rsb uri to publish encoded images.");

  desc.add_options()(
      "encoding,e",
      boost::program_options::value<std::string>()->default_value("jpg"),
      "The output encoding to use. Can be on of ( none | ppm | png | jpg | jp2 "
      "| tiff ). Is set to none, this application produces the usual "
      "rst::vision::Image data.");

  desc.add_options()("scale-width,x",
                     boost::program_options::value<double>()->default_value(1.),
                     "Scale the output image-width by the passed factor.");

  desc.add_options()("scale-height,y",
                     boost::program_options::value<double>()->default_value(1.),
                     "Scale the output image-height by the passed factor.");

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
  const std::string out_scope = program_options["output-uri"].as<std::string>();
  const double scale_width = program_options["scale-width"].as<double>();
  const double scale_height = program_options["scale-height"].as<double>();
  if (scale_height <= 0 || scale_width <= 0) {
    std::cerr << "Cannot scale images with a factor of 0 or less.";
    return 1;
  }
  ScaleImageOpenCV scale(scale_width, scale_height);
  auto in = std::make_shared<ImageListener>(in_scope);

  const std::string encoding = program_options["encoding"].as<std::string>();
  if (encoding == "none") {
    auto out = std::make_shared<ImageInformer>(out_scope);
    auto connection =
        in->connect([&scale, &out](ImageListener::DataType image) {
          out->publish(scale.scale(image.data()),image.causes());
        });
    block();

  } else {
    const ImageEncoding::Type encoder = ImageEncoding::stringToType(
        program_options["encoding"].as<std::string>());

    auto out = std::make_shared<EncodedImageInformer>(out_scope);

    EncodeRstVisionImage compress(encoder);
    auto connection =
        in->connect([&scale, &compress, &out](ImageListener::DataType image) {
          out->publish(compress.encode(scale.scale(image.data())),image.causes());
        });
    block();
  }

}
