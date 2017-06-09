/********************************************************************
**                                                                 **
** File   : src/utils/Subject.h                                    **
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

#include <boost/signals2/signal.hpp>
#include <functional>

namespace pontoon {
namespace utils {

template <typename Data> class Subject : public boost::noncopyable {
private:
  typedef boost::signals2::signal<void(Data)> Signal;

public:
  typedef boost::signals2::connection Connection;
  typedef Data DataType;
  typedef std::shared_ptr<Subject<Data>> Ptr;

  Subject() = default;
  virtual ~Subject() = default;

  Connection connect(std::function<void(Data)> subscriber) {
    return _Signal.connect(subscriber);
  }

  void disconnect(Connection subscriber) { subscriber.disconnect(); }

  void notify(Data data) { _Signal(data); }

private:
  Signal _Signal;
};

template <typename Data> class CompositeSubject : public Subject<Data> {
public:
  CompositeSubject(const std::vector<typename Subject<Data>::Ptr> subjects)
      : _Subjects(subjects) {
    for (auto s : subjects) {
      _Connections.push_back(
          s->connect([this](Data data) { this->notify(data); }));
    }
  }

  virtual ~CompositeSubject() {
    for (auto c : _Connections) {
      c.disconnect();
    }
  }

private:
  std::vector<typename Subject<Data>::Ptr> _Subjects;
  std::vector<typename Subject<Data>::Connection> _Connections;
};

} // namespace utils
} // namespace pontoon
