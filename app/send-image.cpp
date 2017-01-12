/********************************************************************
**                                                                 **
** File   : app/send-image.cpp                                     **
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
#include <iostream>

#include <convert/ConvertRstImageOpenCV.h>
#include <io/ImageIO.h>
#include <io/rst/Informer.h>
#include <io/rst/InformerCVImage.h>

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description = "This application reads a single image from a file "
                            "and publishes it via rsb.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message")

      ("file,f", boost::program_options::value<std::string>(),
       "The file to send.")

          ("output-uri,o",
           boost::program_options::value<std::string>()->default_value(
               "/image"),
           "The output rsb uri to send the image to.")

              ("type,t",
               boost::program_options::value<std::string>()->default_value(
                   "jpg"),
               "The output type to use. Can be on of ( img | bmp | ppm | png | "
               "jpg | jp2 | tiff ). "
               "The send image data is rst::vision::Image in case of 'img' and "
               "rst::vision::EncodedImage "
               "in all other cases.")

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
  const std::string type = program_options["type"].as<std::string>();
  const std::string file = program_options["file"].as<std::string>();

  // send image
  auto image = pontoon::io::ImageIO::readImage(file);
  if (type == "img") {
    pontoon::io::rst::InformerCVImage informer(out_uri);
    informer.publish(image);
  } else {
    pontoon::convert::EncodeRstVisionImage encoder(
        pontoon::convert::ImageEncoding::stringToType(type));
    pontoon::io::rst::Informer<rst::vision::EncodedImage> informer(out_uri);
    informer.publish(encoder.encode(image));
  }
}
