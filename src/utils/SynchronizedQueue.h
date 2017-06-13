/********************************************************************
**                                                                 **
** File   : src/utils/SynchronizedQueue.h                          **
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

#include <condition_variable>
#include <mutex>
#include <queue>

namespace pontoon {
namespace utils {

template <typename Data> class SynchronizedQueue {
public:
  typedef std::mutex Mutex;
  typedef std::unique_lock<Mutex> Lock;
  typedef std::condition_variable ConditionVariable;
  typedef Data DataType;
  typedef std::chrono::milliseconds Milliseconds;

  SynchronizedQueue(size_t maximum_size = -1) : max_size(maximum_size) {}

  ~SynchronizedQueue() {
    Lock lock(mutex);
    exit = true;
    condition.notify_all();
  }

  void push(Data const &data) {
    Lock lock(mutex);
    if (max_size > 0 && queue.size() > max_size) {
      queue.pop();
    }
    queue.push(data);
    lock.unlock();
    condition.notify_one();
  }

  bool empty() const {
    Lock lock(mutex);
    return queue.empty();
  }

  bool try_pop(Data &popped_value) {
    Lock lock(mutex);
    if (queue.empty()) {
      return false;
    }
    popped_value = queue.front();
    queue.pop();
    return true;
  }

  bool try_pop_for(Data &popped_value, Milliseconds& duration) {
    auto until = std::chrono::system_clock::now() + duration;
    Lock lock(mutex);
    while(!exit && queue.empty() && std::chrono::system_clock::now() < until){
      condition.wait_until(lock,until);
    }
    if(!queue.empty()){
      popped_value = queue.front();
      queue.pop();
      return true;
    } else {
      return false;
    }
  }

  void pop(Data &data) {
    Lock lock(mutex);
    while (queue.empty()) {
      if (exit) {
        return;
      }
      condition.wait(lock);
    }
    data = queue.front();
    queue.pop();
  }

private:
  std::queue<Data> queue;
  mutable Mutex mutex;
  ConditionVariable condition;
  size_t max_size;
  bool exit = false;
};

} // namespace utils
} // namespace pontoon
