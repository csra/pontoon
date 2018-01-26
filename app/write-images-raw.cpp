/********************************************************************
**                                                                 **
** File   : app/write-images-raw.cpp                               **
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
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <fstream>
#include <mutex>
#include <rsb/MetaData.h>
#include <thread>

class CapturedFrame;

using ImageListener = pontoon::io::rst::CombinedCVImageListener;
using ImageQueue = pontoon::utils::SynchronizedQueue<CapturedFrame>;
using time_delta = uint64_t;
using pontoon::io::rst::EventData;

class CapturedFrame {
private:
  boost::shared_ptr<cv::Mat> _frame;
  size_t _frame_number;
  time_delta _start_time;
  time_delta _frame_time;

public:
  CapturedFrame() {}

  CapturedFrame(boost::shared_ptr<cv::Mat> data, size_t frame_number,
                time_delta start_time, time_delta frame_time)
      : _frame(data), _frame_number(frame_number), _start_time(start_time),
        _frame_time(frame_time) {}

  bool valid() const { return _frame.get() != nullptr; }

  const cv::Mat &image() const { return *_frame; }

  const char *data() const { return (char *)&_frame->data[0]; }

  size_t num_bytes() const { return _frame->total() * _frame->elemSize(); }

  time_delta start_time() const { return _start_time; }

  time_delta frame_time() const { return _frame_time; }

  size_t frame_number() const { return _frame_number; }
};

class FrameDumper {
private:
  std::ofstream of_video;
  std::ofstream of_timestamps;
  size_t io_buffer_size;

public:
  FrameDumper(const std::string &video_dst, const std::string &timestamp_dst,
              size_t buffer_size)
      : io_buffer_size(buffer_size) {
    if (!video_dst.empty()) {
      of_video.open(video_dst, std::ofstream::binary);
    }
    if (!timestamp_dst.empty()) {
      of_timestamps.open(timestamp_dst);
      of_timestamps << std::fixed << "clock\t# timecode format v2" << std::endl;
    }
    if (!of_video.is_open()) {
      std::cerr << "WARNING: could not open video output file. will not write "
                   "image data."
                << std::endl;
    }
    if (!of_timestamps.is_open()) {
      std::cerr << "WARNING: could not open timestamps output file. will not "
                   "write timestamp data."
                << std::endl;
    }
  }

  void dump_frame(const CapturedFrame &frame) {
    if (of_video.is_open()) {
      dump_buffered(frame.data(), of_video, frame.num_bytes(), io_buffer_size);
    }
    if (of_timestamps.is_open()) {
      of_timestamps << frame.frame_time() / 1e6 << '\t'
                    << (frame.frame_time() - frame.start_time()) / 1e3
                    << std::endl;
    }
  }

private:
  void dump_buffered(const char *data, std::ofstream &dst, size_t data_size,
                     size_t buffer_size) {
    if (buffer_size > 0) {
      for (size_t i = 0; i < data_size / buffer_size; i++) {
        dst.write(data, buffer_size);
        data += buffer_size;
      }
      dst.write(data, data_size % buffer_size);
    } else {
      dst.write(data, data_size);
    }
  }
};

class Statistics {
private:
  using time_accumulator = boost::accumulators::accumulator_set<
      time_delta,
      boost::accumulators::stats<boost::accumulators::tag::rolling_sum>>;
  using size_accumulator = boost::accumulators::accumulator_set<
      size_t,
      boost::accumulators::stats<boost::accumulators::tag::rolling_sum>>;
  using rolling_mean = boost::accumulators::tag::rolling_mean;
  using time_point = std::chrono::high_resolution_clock::time_point;

  const size_t window_size = 100;
  const size_t bytes_in_kbytes = std::pow(2, 10);
  const size_t bytes_in_mbytes = std::pow(2, 20);

  size_t _counter;
  std::chrono::high_resolution_clock::time_point _timestamp;
  time_delta _last_frame_time;
  time_accumulator _fps;
  time_accumulator _original_fps;
  size_accumulator _bps;

public:
  Statistics()
      : _counter(0), _timestamp(std::chrono::high_resolution_clock::now()),
        _last_frame_time(0), _fps(rolling_mean::window_size = window_size),
        _original_fps(rolling_mean::window_size = window_size),
        _bps(rolling_mean::window_size = window_size) {}

  void update(const CapturedFrame &frame) {
    time_point now = std::chrono::high_resolution_clock::now();
    _fps((now - _timestamp).count());
    _original_fps(frame.frame_time() - _last_frame_time);
    _timestamp = now;
    _last_frame_time = frame.frame_time();
    _bps(frame.num_bytes());
    if (++_counter % window_size == 0) {
      auto fps = 1e11/boost::accumulators::rolling_sum(_fps);
      auto ofps = 1e8/boost::accumulators::rolling_sum(_original_fps);
      auto bps = boost::accumulators::rolling_sum(_bps) / window_size * fps;
      std::cerr << "frames: " << _counter << "(# " << frame.frame_number() << ")"
                << "\n           fps: " << std::left
                << std::setw(6) << fps << "\n  original fps: " << std::left
                << std::setw(6) << ofps << "\n          mbps: " << std::left
                << std::setw(6) << bps / bytes_in_mbytes << std::endl;
    }
  }
};

int main(int argc, char **argv) {
  boost::program_options::variables_map program_options;

  std::string description =
      "This application listens for images and dumps them into a file.";
  std::stringstream description_text;
  description_text << description << "\n\n"
                   << "Allowed options";
  boost::program_options::options_description desc(description_text.str());
  desc.add_options()("help,h", "produce help message");

  desc.add_options()("input-uri,i", boost::program_options::value<std::string>()
                                        ->default_value("/video/compressed"),
                     "The input rsb uri to receive rst::vision::Image or "
                     "rstexperimental::vision::EncodedImage.");

  desc.add_options()(
      "output-file-name,o",
      boost::program_options::value<std::string>()->default_value(""),
      "The name of the file to dump images to.");

  desc.add_options()(
      "timestamp-file-name,t",
      boost::program_options::value<std::string>()->default_value(""),
      "The name of the file to dump timestamp information to.");

  desc.add_options()(
      "buffer-size-image-out,b",
      boost::program_options::value<size_t>()->default_value(32768),
      "The buffer size to use when writing images.");

  desc.add_options()(
      "max-queue-size,m",
      boost::program_options::value<size_t>()->default_value(150),
      "How many images to hold before starting to drop frames.");

  desc.add_options()(
      "sanity-kill,s",
      boost::program_options::value<size_t>()->default_value(0),
      "Stop the application if no new images arrive for passed amount of milliseconds. 0 for never.");


  desc.add_options()("print-statistics,p", "Print statistics to std::err.");

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
  const std::string image_dst =
      program_options["output-file-name"].as<std::string>();
  const std::string timestamp_dst =
      program_options["timestamp-file-name"].as<std::string>();
  const size_t queue_size = program_options["max-queue-size"].as<size_t>();
  const size_t out_buffer_size =
      program_options["buffer-size-image-out"].as<size_t>();
  const bool print_stats = program_options.count("print-statistics") > 0;
  const auto sanity_kill_millis = program_options["sanity-kill"].as<size_t>();

  // init components
  std::mutex mutex;

  ImageListener image_listener(in_scope);
  ImageQueue queue(queue_size);

  bool first = true;
  time_delta start_time = 0;

  auto collect_images = image_listener.connect(
      [&queue, &start_time, &first](ImageListener::DataType image) {
        if (!image.valid()) {
          return;
        }
        if (first) {
          start_time = image.event()->getMetaData().getCreateTime();
          first = false;
        }
        auto frame_time = image.event()->getMetaData().getCreateTime();
        queue.push(CapturedFrame(image.data(),
                                 image.event()->getId().getSequenceNumber(),
                                 start_time, frame_time));
      });

  std::cerr << "Ready..." << std::endl;

  FrameDumper dumper(image_dst, timestamp_dst, out_buffer_size);
  Statistics stats;
  for (;;) {
    ImageQueue::DataType frame;
    if (sanity_kill_millis > 0 && !queue.try_pop_for(frame, std::chrono::milliseconds(sanity_kill_millis))) {
      std::cerr << "Could not get an image for " << sanity_kill_millis << "ms. Leaving application." << std::endl;
      break;
    } else {
      queue.pop(frame);
    }
    if (frame.valid()) {
      dumper.dump_frame(frame);
    }
    if (print_stats) {
      stats.update(frame);
    }
  }
}
