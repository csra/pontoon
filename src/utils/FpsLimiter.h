/********************************************************************
**                                                                 **
** File   : src/utils/FpsLimiter.h                                 **
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

#pragma once

#include <mutex>
#include <thread>

namespace pontoon {
namespace utils {

class FpsLimiter {
public:
  typedef std::mutex Mutex;
  typedef std::unique_lock<Mutex> Lock;
  typedef std::chrono::microseconds Microseconds;
  typedef std::chrono::system_clock::time_point TimePoint;

  FpsLimiter(double max_fps = -1.) {
    if (max_fps <= 0) {
      _frame_time = Microseconds(0);
    } else {
      _frame_time = Microseconds(size_t(1e6 / max_fps));
    }
    _last_wait = now();
  }

  ~FpsLimiter() { _exit = true; }

  void wait() {
    while (!_exit && (now() < (_last_wait + _frame_time))) {
      std::this_thread::sleep_until(_last_wait + _frame_time);
    }
    _last_wait = now();
  }

  static TimePoint now() { return std::chrono::system_clock::now(); }

private:
  Mutex _mutex;
  Microseconds _frame_time;
  TimePoint _last_wait;
  bool _exit = false;
};

} // namespace utils
} // namespace pontoon
