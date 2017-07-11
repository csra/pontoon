/********************************************************************
**                                                                 **
** File   : app/cut-faces.cpp                                      **
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

#include "convert/ConvertRstImageOpenCV.h"
#include "convert/ScaleImageOpenCV.h"
#include "io/rst/Informer.h"
#include "io/rst/InformerCVImage.h"
#include "io/rst/ListenerCVImage.h"
#include "utils/CvHelpers.h"
#include <boost/program_options.hpp>
#include <mutex>
#include <rst/vision/FaceWithGazeCollection.pb.h>
#include <rst/vision/Faces.pb.h>

typedef pontoon::io::rst::CombinedCVImageListener ImageListener;
typedef pontoon::io::rst::EncodingMultiImageInformer ImageInformer;
using pontoon::convert::ImageEncoding;
using pontoon::convert::ScaleImageOpenCV;
using pontoon::convert::EncodeRstVisionImage;
using pontoon::utils::Subject;
using rst::vision::Face;
using rst::vision::FaceWithGazeCollection;
using rst::vision::Faces;
typedef pontoon::io::rst::EventData<IplImage> ImageData;

struct FaceData {
  rsb::EventPtr event;
  std::vector<Face> faces;

  FaceData(pontoon::io::rst::EventData<FaceWithGazeCollection> &data)
      : event(data.event()) {
    for (auto facewithgaze : data.data()->element()) {
      if (facewithgaze.has_region()) {
        faces.push_back(facewithgaze.region());
      }
    }
  }

  FaceData(pontoon::io::rst::EventData<Faces> &data) : event(data.event()) {
    for (auto face : data.data()->faces()) {
      if (face.has_region()) {
        faces.push_back(face);
      }
    }
  }
};

struct ImageAndFaceData {
  FaceData faces;
  ImageData image;
  pontoon::io::Causes causes;

  ImageAndFaceData(FaceData _faces, ImageData _image)
      : faces(_faces), image(_image),
        causes{faces.event->getId(), image.event()->getId()} {}
};

class ImageFaceListener : public Subject<ImageAndFaceData> {
public:
  ImageFaceListener(const std::string &img_uri, const std::string &face_uri)
      : _imageListener(img_uri), _facesListener1(face_uri),
        _facesListener2(face_uri) {
    _imageListener.connect([this](ImageData data) {
      if (data.valid()) {
        this->_images.push_back(data);
        this->update();
      }
    });
    _facesListener1.connect(
        [this](pontoon::io::rst::EventData<FaceWithGazeCollection> data) {
          if (data.valid()) {
            this->_faces.push_back(FaceData(data));
            this->update();
          }
        });
    _facesListener2.connect([this](pontoon::io::rst::EventData<Faces> data) {
      if (data.valid()) {
        this->_faces.push_back(FaceData(data));
        this->update();
      }
    });
  }

private:
  void update() {
    for (size_t i = 0; i < _faces.size(); ++i) {
      FaceData &face = _faces[i];
      for (size_t j = 0; j < _images.size(); ++j) {
        ImageData &image = _images[j];
        if (face.event->getCauses().find(image.event()->getId()) !=
            face.event->getCauses().end()) {
          // found faces corresponding to image
          this->notify(ImageAndFaceData(face, image));
          // cleanup
          _faces.erase(_faces.begin() + i);
          _images.erase(_images.begin() + j);
          while (_faces.size() > _max_size) {
            _faces.pop_front();
          }
          while (_images.size() > _max_size) {
            _images.pop_front();
          }
        }
      }
    }
  }

  ImageListener _imageListener;
  pontoon::io::rst::Listener<FaceWithGazeCollection> _facesListener1;
  pontoon::io::rst::Listener<Faces> _facesListener2;
  std::deque<ImageData> _images;
  std::deque<FaceData> _faces;
  size_t _max_size = 10;
};

std::vector<boost::shared_ptr<IplImage>>
cut_faces(const ImageAndFaceData &data) {
  std::vector<boost::shared_ptr<IplImage>> result;
  cv::Mat image = cv::cvarrToMat(data.image.data().get());
  for (auto face : data.faces.faces) {
    if (face.has_region()) {
      auto x = face.region().top_left().x();
      auto y = face.region().top_left().y();
      auto w = face.region().width();
      auto h = face.region().height();
      cv::Rect roi(x, y, w, h);
      auto patch = boost::make_shared<cv::Mat>(image(roi));
      result.push_back(pontoon::utils::cvhelpers::asIplImagePtr(patch));
    }
  }
  return result;
}

void block() {
  std::cerr << "Ready..." << std::endl;
  // deadlock
  std::mutex lock;
  lock.lock();
  lock.lock();
}

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description = "Listens for images and face detections. Pulishes "
                            "the face patches corresponding to detected faces.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()(
      "image-uri,i",
      boost::program_options::value<std::string>()->default_value("/video/raw"),
      "The input rsb uri to receive images.");

  desc.add_options()("faces-uri,f", boost::program_options::value<std::string>()
                                        ->default_value("/video/faces"),
                     "The input rsb uri to receive face detection results.");

  desc.add_options()(
      "output-uri,o",
      boost::program_options::value<std::string>()->default_value(
          "/video/facepatches"),
      "The output rsb uri to publish collections of face patches.");

  desc.add_options()(
      "encoding,e",
      boost::program_options::value<std::string>()->default_value("jpg"),
      "The output encoding to use. Can be on of ( none | ppm | png | jpg | jp2 "
      "| tiff ). Is set to none, this application produces the usual "
      "rst::vision::Image data.");

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

  const std::string image_scope =
      program_options["image-uri"].as<std::string>();
  const std::string faces_scope =
      program_options["faces-uri"].as<std::string>();
  const std::string out_scope = program_options["output-uri"].as<std::string>();
  const std::string encoding = program_options["encoding"].as<std::string>();

  auto in = std::make_shared<ImageFaceListener>(image_scope, faces_scope);
  auto out = std::make_shared<ImageInformer>(out_scope, encoding);

  auto connection = in->connect([&in, &out](ImageAndFaceData data) {
    out->publish(cut_faces(data), data.causes);
  });

  block();
}
