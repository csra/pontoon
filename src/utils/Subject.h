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

#include <functional>
#include <boost/signals2/signal.hpp>

namespace utils {

template<typename Data>
class Subject : public boost::noncopyable
{
private:
    typedef boost::signals2::signal<void (Data)> Signal;
public:
    typedef boost::signals2::connection  Connection;
    typedef Data DataType;
    typedef std::shared_ptr<Subject<Data>> Ptr;

    Subject() = default;
    virtual ~Subject() = default;

    Connection connect(std::function<void (Data)> subscriber) {
      return m_Signal.connect(subscriber);
    }

    void disconnect(Connection subscriber) {
      subscriber.disconnect();
    }

    void notify(Data data) {
      m_Signal(data);
    }

private:
    Signal m_Signal;
};

} // namespace utils
