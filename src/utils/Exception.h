/********************************************************************
**                                                                 **
** File   : src/utils/Exception.h                                  **
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

#include <exception>
#include <string>

namespace pontoon {
namespace utils {

class Exception : public std::exception {
public:
  Exception(const std::string& error) : message(error){}

  virtual const char* what() const noexcept override { return message.c_str(); }


private:
  const std::string message;
};

} // namespace utils
} // namespace pontoon
