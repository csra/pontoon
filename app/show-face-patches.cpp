/********************************************************************
**                                                                 **
** File   : app/show-face-patches.cpp                              **
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

#include "io/rst/ListenerCVImage.h"
#include "io/rst/ListenerFaces.h"
#include "utils/FpsLimiter.h"
#include "utils/SynchronizedQueue.h"
#include <atomic>
#include <boost/program_options.hpp>
#include <memory>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using ImageListener = pontoon::io::rst::CombinedCVImageListener;
using FacePatchesListener =
    pontoon::io::rst::ListenerCVImageRstEncodedImageCollection;
using FacesListener = pontoon::io::rst::ListenerFaces;
using pontoon::utils::FpsLimiter;

auto allEventsHaveTheSameCauseComparator =
    [](const std::vector<pontoon::io::rst::EventDataBase *> &events) -> bool {
  if (events.empty()) {
    return false;
  }
  pontoon::io::Causes causes = events.front()->causes();
  for (auto it = events.begin() + 1; it != events.end(); ++it) {
    std::vector<pontoon::io::Cause> intersection;
    std::set_intersection(causes.begin(), causes.end(), (*it)->causes().begin(),
                          (*it)->causes().end(),
                          std::back_inserter(intersection));
    if (!intersection.empty()) {
      causes.clear();
      causes.insert(intersection.begin(), intersection.end());
    } else {
      return false;
    }
  }
  return true;
};

class FaceAndPatchListener
    : public pontoon::utils::Subject<
          std::tuple<FacesListener::DataType, FacePatchesListener::DataType>> {
public:
  using FacesData = FacesListener::DataType;
  using FacePatchesData = FacePatchesListener::DataType;

private:
  FacesListener _facesListener;
  FacePatchesListener _facePatchesListener;
  std::deque<FacesData> _faces;
  std::deque<FacePatchesData> _images;
  size_t _max_size = 10;
  std::mutex _mutex;

public:
  FaceAndPatchListener(const std::string &faces_uri,
                       const std::string &face_patches_uri)
      : _facesListener(faces_uri), _facePatchesListener(face_patches_uri) {
    _facesListener.connect([this](FacesData data) {
      if (data.valid()) {
        std::lock_guard<std::mutex> lock(this->_mutex);
        this->_faces.push_back(data);
        this->update();
      }
    });
    _facePatchesListener.connect([this](FacePatchesData data) {
      if (data.valid()) {
        std::lock_guard<std::mutex> lock(this->_mutex);
        this->_images.push_back(data);
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
        FacePatchesData &image = *image_it;
        if (allEventsHaveTheSameCauseComparator({&face, &image})) {
          // found faces corresponding to image
          this->notify(std::make_tuple(face, image));
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
};

cv::Size getFacesImageSize(const FacesListener::DataType::DataType &faces) {
  cv::Size size(0, 0);
  for (auto face : faces) {
    if (face->region().has_image_width() && face->region().has_image_width()) {
      cv::Size face_size(face->region().image_width(),
                         face->region().image_height());
      if (size.area() != 0 && face_size != size) {
        std::cerr << "ERROR: faces have different image sizes." << std::endl;
        break;
      } else {
        size = face_size;
      }
    }
  }
  return size;
}

void scaleAndReplaceImage(std::unique_ptr<cv::Mat> &dst,
                          ImageListener::DataType &src,
                          FacesListener::DataType &faces) {
  if (!src.valid()) {
    return;
  }
  const cv::Mat &tmp = *src.data().get();
  cv::Size size = getFacesImageSize(faces.data());
  if (size.area() == 0) {
    size = cv::Size(src.data()->cols, src.data()->rows);
  }
  if (tmp.cols != size.width || tmp.rows != size.height) {
    cv::resize(tmp, *dst, size, 0, 0, cv::INTER_LINEAR);
  } else {
    dst.reset(new cv::Mat(tmp.clone()));
  }
}

cv::Rect faceToRoi(const rst::vision::Face &face) {
  return cv::Rect(face.region().top_left().x(), face.region().top_left().y(),
                  face.region().width(), face.region().height());
}

void paintPatches(std::unique_ptr<cv::Mat> &dst, FacesListener::DataType &faces,
                  FacePatchesListener::DataType &patches) {
  if (!faces.valid()) {
    return;
  }
  for (size_t i = 0; i < faces.data().size(); ++i) {
    if (patches.data().size() <= i) {
      std::cerr << "ERROR: less face patches than face recognitions."
                << std::endl;
      continue;
    }
    cv::Rect roi = faceToRoi(*faces.data().at(i));
    const cv::Mat &patch = *patches.data().at(i).get();
    if (cv::Size2i(patch.cols, patch.rows) != roi.size()) {
      std::cerr << "ERROR: face patch and face detection #" << i
                << " have different sizes." << std::endl;
      continue;
    }
    if (roi.x + roi.width > dst->cols || roi.y + roi.height > dst->rows) {
      std::cerr << "ERROR: face patch is bigger than the background image."
                << std::endl;
      continue;
    }
    cv::Mat roi_in_dst = (*dst)(roi);
    patch.copyTo(roi_in_dst);
  }
}

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description =
      "Shows face patches on the corresponding background image.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()(
      "patches-uri,p",
      boost::program_options::value<std::string>()->default_value("/patches"),
      "The input rsb uri to receive rst::vision::EncodedImageCollections as "
      "face patches.");

  desc.add_options()(
      "faces-uri,f",
      boost::program_options::value<std::string>()->default_value("/faces"),
      "The input rsb uri to receive rst::vision::Faces or "
      "rst::vision::FaceWithGazeCollections this is used to set the face "
      "patches to the correct positions in the image.");

  desc.add_options()(
      "background-uri,b",
      boost::program_options::value<std::string>()->default_value("/video"),
      "The input rsb uri to receive images to use as background for the face "
      "patches.");

  desc.add_options()("fps",
                     boost::program_options::value<double>()->default_value(30),
                     "The maximum amount of frames to show per second");

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

  auto image_uri = program_options["background-uri"].as<std::string>();
  auto face_uri = program_options["faces-uri"].as<std::string>();
  auto patch_uri = program_options["patches-uri"].as<std::string>();

  ImageListener image_listener(image_uri);
  FaceAndPatchListener face_patches_listener(face_uri, patch_uri);

  std::mutex mutex;
  ImageListener::DataType image;
  FaceAndPatchListener::DataType patches;
  std::atomic_bool refresh(true);

  image_listener.connect(
      [&mutex, &image, &refresh](ImageListener::DataType data) {
        std::lock_guard<std::mutex> lock(mutex);
        image = data;
        std::cerr << "image received" << std::endl;
        refresh.store(true);
      });

  face_patches_listener.connect(
      [&mutex, &patches, &refresh](FaceAndPatchListener::DataType data) {
        std::lock_guard<std::mutex> lock(mutex);
        patches = data;
        std::cerr << "patches received" << std::endl;
        refresh.store(true);
      });

  std::string window_name("pontoon-face-patches");
  cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
  int key = -1;
  std::unique_ptr<cv::Mat> view(new cv::Mat(0, 0, CV_8UC3));
  FpsLimiter fps(program_options["fps"].as<double>());
  size_t frame = 0;
  while (key != 27) {
    if (refresh.load()) {
      mutex.lock();
      ImageListener::DataType image_copy = image;
      FaceAndPatchListener::DataType patches_copy = patches;
      scaleAndReplaceImage(view, image_copy, std::get<0>(patches_copy));
      paintPatches(view, std::get<0>(patches_copy), std::get<1>(patches_copy));
      refresh.store(false);
      mutex.unlock();
      if (!view->empty()) {
        std::cout << "frame" << ++frame << std::endl;
        cv::imshow(window_name, *view);
      }
    }
    key = cv::waitKey(1);
    fps.wait();
  }
  std::cout << "ESCAPE received. Leaving application." << std::endl;
}
