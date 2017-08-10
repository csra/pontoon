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

#include "io/rst/InformerCVImage.h"
#include "io/rst/ListenerCVImage.h"
#include "io/rst/ListenerFaces.h"
#include "utils/CvHelpers.h"
#include "utils/Subject.h"
#include <boost/program_options.hpp>
#include <mutex>

using ImageInformer = pontoon::io::rst::EncodingMultiImageInformer;
using ImageListener = pontoon::io::rst::CombinedCVImageListener;
using FacesListener = pontoon::io::rst::ListenerFaces;
using ImageData = ImageListener::DataType;
using FacesData = FacesListener::DataType;
using ::rst::vision::Face;

struct ImageAndFaceData {
  FacesData faces;
  ImageData image;
  pontoon::io::Causes causes;

  ImageAndFaceData(FacesData _faces, ImageData _image)
      : faces(_faces), image(_image),
        causes{faces.event()->getId(), image.event()->getId()} {}
};

class ImageFaceListener : public pontoon::utils::Subject<ImageAndFaceData> {
public:
  ImageFaceListener(const std::string &img_uri, const std::string &face_uri)
      : _imageListener(img_uri), _facesListener(face_uri) {
    _imageListener.connect([this](ImageData data) {
      if (data.valid()) {
        std::lock_guard<std::mutex> lock(this->_mutex);
        this->_images.push_back(data);
        this->update();
      }
    });
    _facesListener.connect([this](FacesData data) {
      if (data.valid()) {
        std::lock_guard<std::mutex> lock(this->_mutex);
        this->_faces.push_back(data);
        this->update();
      }
    });
  }

private:
  void update() {
    auto face_it = _faces.begin();
    while (face_it != _faces.end()) {
      FacesData &face = *face_it;
      auto image_it = _images.begin();
      while (image_it != _images.end()) {
        ImageData &image = *image_it;
        if (face.event()->getCauses().find(image.event()->getId()) !=
            face.event()->getCauses().end()) {
          // found faces corresponding to image
          this->notify(ImageAndFaceData(face, image));
          break;
        } else {
          ++image_it;
        }
      }
      if (image_it != _images.end()) {
        // need to remove notified match. increasing the face_iterator
        face_it = _faces.erase(face_it);
        image_it = _images.erase(image_it);
      } else {
        ++face_it;
      }
    }
    while (_faces.size() > _max_size) {
      std::cerr << "dropping old faces frame: ";
      _faces.front().event()->printContents(std::cerr);
      std::cerr << std::endl;
      _faces.pop_front();
    }
    while (_images.size() > _max_size) {
      std::cerr << "dropping old image frame: ";
      _images.front().event()->printContents(std::cerr);
      std::cerr << std::endl;
      _images.pop_front();
    }
  }

  ImageListener _imageListener;
  FacesListener _facesListener;
  std::deque<ImageData> _images;
  std::deque<FacesData> _faces;
  size_t _max_size = 10;
  std::mutex _mutex;
};

bool checkRoi(const cv::Rect &roi, const cv::Mat &mat) {
  if (roi.x < 0 || roi.y < 0 || roi.height < 0 || roi.width < 0) {
    std::cerr << "ERROR: Roi cannot have values < 0 (" << roi << ")"
              << std::endl;
    return false;
  }
  if (roi.x + roi.width > mat.cols || roi.y + roi.height > mat.rows) {
    std::cerr << "ERROR: Roi must be in image (" << roi << ") image ("
              << mat.cols << "x" << mat.rows << ")" << std::endl;
    return false;
  }
  return true;
}

cv::Rect faceToRoi(const Face &face) {
  return cv::Rect(face.region().top_left().x(), face.region().top_left().y(),
                  face.region().width(), face.region().height());
}

std::vector<boost::shared_ptr<cv::Mat>>
cut_faces(const ImageAndFaceData &data) {
  std::vector<boost::shared_ptr<cv::Mat>> result;
  const cv::Mat &image = *data.image.data();
  auto &faces = data.faces.data();
  for (const auto &face : faces) {
    cv::Rect roi = faceToRoi(*face);
    if (checkRoi(roi, image)) {
      auto patch = boost::make_shared<cv::Mat>(image(roi));
      result.push_back(patch);
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
      "The output encoding to use. Can be on of ( none | ppm | png | jpg | "
      "jp2 "
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
