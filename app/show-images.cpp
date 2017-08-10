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
#include "utils/FpsLimiter.h"
#include "utils/SynchronizedQueue.h"
#include <boost/program_options.hpp>
#include <memory>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef pontoon::io::rst::CombinedCVImageListener ImageListener;
typedef pontoon::utils::SynchronizedQueue<ImageListener::DataType> ImageQueue;
using pontoon::utils::FpsLimiter;

class CollectImages {
public:
  CollectImages(std::vector<ImageListener::Ptr> listeners)
      : _listeners(listeners) {
    for (size_t i = 0; i < listeners.size(); ++i) {
      _queues.push_back(std::unique_ptr<ImageQueue>(new ImageQueue(1)));
      _connections.push_back(listeners[i]->connect([this, i](
          ImageListener::DataType image) { this->_queues[i]->push(image); }));
    }
  }

  std::vector<ImageQueue::DataType> try_pop_all() {
    std::vector<ImageQueue::DataType> result;
    for (auto &queue : _queues) {
      ImageQueue::DataType data;
      queue->try_pop(data);
      result.push_back(data);
    }
    return result;
  }

private:
  std::vector<ImageListener::Ptr> _listeners;
  std::vector<std::unique_ptr<ImageQueue>> _queues;
  std::vector<ImageListener::Connection> _connections;
};

class CombineImages {
private: // helper classes
  struct Position {
    cv::Point _topLeft;
    cv::Size _size;

    Position(size_t pos_h = 0, size_t pos_v = 0, size_t width = 0,
             size_t height = 0)
        : _topLeft(pos_h, pos_v), _size(width, height) {}

    bool equals(const Position &other) {
      return other._topLeft == _topLeft && other._size == _size;
    }
  };

private: // members
  CollectImages _collect;
  std::vector<ImageListener::DataType> _lastImages;
  std::vector<bool> _imageUpdated;
  std::vector<cv::Rect> _positions;
  int _rows;
  int _columns;
  int _cell_width;
  int _cell_height;

public:
  CombineImages(std::vector<ImageListener::Ptr> listeners, size_t rows,
                size_t columns)
      : _collect(listeners), _imageUpdated(listeners.size(), false),
        _rows(rows), _columns(columns) {
    assert(size_t(_rows * _columns) >= listeners.size());
    _lastImages.resize(listeners.size());
    _positions.resize(listeners.size());
  }

  void update(std::unique_ptr<cv::Mat> &image) {
    updateImageCache();
    updateImagePositions();
    writeImages(image);
  }

private: // helper functions
  void updateImageCache() {
    std::vector<ImageListener::DataType> update = _collect.try_pop_all();
    for (size_t i = 0; i < update.size(); ++i) {
      _imageUpdated[i] = update[i].valid();
      if (_imageUpdated[i]) {
        _lastImages[i] = update[i];
      }
    }
  }

  void updateImagePositions() {
    // find biggest sizes in images as cell size
    _cell_width = 0;
    _cell_height = 0;
    for (int r = 0; r < _rows; ++r) {
      for (int c = 0; c < _columns; ++c) {
        int pos = (r * c) + c;
        if (size_t(pos) < _lastImages.size() && _lastImages.at(pos).valid()) {
          const IplImage &image = *_lastImages.at(pos).data();
          // update cell sizes from image
          _cell_width = std::max(_cell_width, image.width);
          _cell_height = std::max(_cell_height, image.height);
        }
      }
    }
    // calculate image positions
    for (int r = 0; r < _rows; ++r) {
      for (int c = 0; c < _columns; ++c) {
        int pos = (r * _columns) + c;
        if (size_t(pos) < _lastImages.size() && _lastImages.at(pos).valid()) {
          const IplImage &image = *_lastImages.at(pos).data();
          // place the image into the center of its cell
          size_t upper_left_h =
              r * _cell_height + (_cell_height - image.height) / 2;
          size_t upper_left_v =
              c * _cell_width + (_cell_width - image.width) / 2;
          _positions.at(pos) = cv::Rect(cv::Point(upper_left_v, upper_left_h),
                                        cv::Size(image.width, image.height));
        }
      }
    }
  }

  void writeImages(std::unique_ptr<cv::Mat> &dst) {
    cv::Size required_dst_size(_columns * _cell_width, _rows * _cell_height);
    if (dst->size() != required_dst_size) {
      dst.reset(new cv::Mat(required_dst_size, dst->type()));
      dst->setTo(0);
    }
    for (size_t i = 0; i < _lastImages.size(); ++i) {
      if (_imageUpdated[i] && _lastImages[i].valid()) {
        const cv::Mat &src = *_lastImages.at(i).data();
        cv::Mat roi = (*dst)(_positions.at(i));
        src.copyTo(roi);
      }
    }
  }
};

void fix_grid_first(size_t &first, size_t &second, size_t sum) {
  if (second >= sum) {
    second = sum;
    first = 1;
    return;
  }
  first = sum / second;
  if (first * second < sum) {
    ++first;
  }
}

void fix_grid(size_t &rows, size_t &cols, size_t sum) {
  if (rows == 0 && cols == 0) {
    rows = std::sqrt(sum);
    fix_grid_first(cols, rows, sum);
    return;
  } else if (cols == 0) {
    fix_grid_first(cols, rows, sum);
  } else {
    fix_grid_first(rows, cols, sum);
  }
}

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
      boost::program_options::value<std::vector<std::string>>()->default_value(
          {"/video"}),
      "The input rsb uri to receive rst::vision::Image or "
      "rstexperimental::vision::EncodedImage. Can be provided "
      "multiple times.");

  desc.add_options()("rows,r",
                     boost::program_options::value<size_t>()->default_value(0),
                     "How many rows to create");

  desc.add_options()("cols,c",
                     boost::program_options::value<size_t>()->default_value(0),
                     "How many columns to create");

  desc.add_options()("fps,f",
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

  const std::vector<std::string> in_scopes =
      program_options["input-uri"].as<std::vector<std::string>>();

  std::vector<ImageListener::Ptr> listeners;
  for (auto scope : in_scopes) {
    listeners.push_back(std::make_shared<ImageListener>(scope));
  }

  auto rows = program_options["rows"].as<size_t>();
  auto cols = program_options["cols"].as<size_t>();
  fix_grid(rows, cols, listeners.size());

  CombineImages combine(listeners, rows, cols);

  std::string window_name("pontoon-show-images");
  cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
  int key = -1;
  std::unique_ptr<cv::Mat> image(new cv::Mat(0, 0, CV_8UC3));
  FpsLimiter fps(program_options["fps"].as<double>());
  size_t frame = 0;
  while (key != 27) {
    combine.update(image);
    if (!image->empty()) {
      cv::imshow(window_name, *image);
    }
    key = cv::waitKey(1);
    fps.wait();
    std::cout << "frame" << ++frame << std::endl;
  }
  std::cout << "ESCAPE received. Leaving application." << std::endl;
}
