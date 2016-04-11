/********************************************************************
**                                                                 **
** File   : app/compress-images.cpp                                **
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

#include <io/rst/Informer.h>
#include <io/rst/ListenerCVImage.h>
#include <convert/ConvertRstImageOpenCV.h>

typedef io::rst::ListenerCVImage ImageListener;
typedef io::rst::Informer<rstexperimental::vision::EncodedImage> ImageInformer;



int main(int argc, char **argv){
  boost::program_options::variables_map program_options;

  std::string description = "This application listens for published images and publishes compressed versions.";
  std::stringstream description_text;
  description_text << description << "\n\n" << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()
      ("help,h","produce help message")

      ("input-url,i",
       boost::program_options::value<std::string>()->default_value("/video/raw"),
       "The input rsb url to receive raw images.")

      ("output-url,o",
       boost::program_options::value<std::string>()->default_value("/video/compressed"),
       "The output rsb url to publish compressed images.")

      ("encoding,e",
       boost::program_options::value<std::string>()->default_value("jpg"),
       "The output encoding to use. Can be on of ( bmp | ppm | png | jpg | jp2 | tiff ).")

      ;

  try {
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), program_options);
    boost::program_options::notify(program_options);

    std::stringstream arguments;
    for(int i = 0; i < argc; ++i){
      arguments << argv[i] << " ";
    }
    std::cerr << "Program started with line: " << arguments.str() << std::endl;

    if (program_options.count("help")) {
      std::cout << desc << "\n";
      return 1;
    }

  } catch (boost::program_options::error& e) {
    std::stringstream arguments;
    for(int i = 0; i < argc; ++i){
      arguments << argv[i] << " ";
    }
    std::cerr << "Could not parse program options: " << e.what();
    std::cerr << "\n\n" << desc << "\n";
    return 1;
  }

  const std::string  in_scope = program_options["input-url" ].as<std::string>();
  const std::string out_scope = program_options["output-url"].as<std::string>();

  const convert::ConvertRstImageOpenCV::Type encoding =  convert::ConvertRstImageOpenCV::stringToType(
      program_options["encoding"].as<std::string>());

  // init rsb components
  auto in = std::make_shared<ImageListener>(in_scope);
  auto out = std::make_shared<ImageInformer>(out_scope);

  convert::ConvertRstImageOpenCV compress(encoding);
  auto connection = in->connect([&compress, &out] (ImageListener::DataPtr image) {
                                      out->publish(compress.encode(image));
                                    });

  std::cerr << "Ready..." << std::endl;

  // deadlock
  std::mutex lock;
  lock.lock();
  lock.lock();
}
