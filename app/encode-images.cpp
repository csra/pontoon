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

#include "io/rst/InformerCVImage.h"
#include "io/rst/ListenerCVImage.h"
#include <boost/program_options.hpp>
#include <mutex>

typedef pontoon::io::rst::CombinedCVImageListener ImageListener;
typedef pontoon::io::rst::EncodingImageInformer ImageInformer;
using pontoon::utils::Subject;

template <typename T> class SkippingSubject : public Subject<T> {
public:
  SkippingSubject(typename Subject<T>::Ptr source, double skip)
      : _source(source) {
    _received = skip;
    _connection = source->connect([this, skip](T data) {
      this->_received += 1.;
      if (this->_received >= skip) {
        this->_received -= skip;
        this->notify(data);
      }
    });
  }

private:
  typename Subject<T>::Ptr _source;
  typename Subject<T>::Connection _connection;
  double _received;
};

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

  desc.add_options()("skip-frames,s",
                     boost::program_options::value<double>()->default_value(0.),
                     "The rate of received images to be ignored. Can be used "
                     "to reduce frame rate.");

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
  const double skip_frames = program_options["skip-frames"].as<double>();

  if (scale_height <= 0 || scale_width <= 0) {
    std::cerr << "Cannot scale images with a factor of 0 or less.";
    return 1;
  }

  auto in = std::make_shared<SkippingSubject<ImageListener::DataType>>(
      std::make_shared<ImageListener>(in_scope), skip_frames);

  ImageInformer out(out_scope, program_options["encoding"].as<std::string>(),
                    scale_width, scale_height);
  auto connection = in->connect([&out](ImageListener::DataType data) {
    out.publish(data.data(), {data.id()});
  });
}
