/********************************************************************
**                                                                 **
** File   : src/convert/CompressRstImageZlib.cpp                   **
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

#include "convert/CompressRstImageZlib.h"
#include "utils/Exception.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>
#include <zlib.h>

using pontoon::convert::CompressRstImageZlib;

std::string error_code(int code) {
  switch (code) {
  case Z_OK:
    return "Ok";
  case Z_MEM_ERROR:
    return "MemoryError";
  case Z_BUF_ERROR:
    return "BufferError";
  default:
    return "__unknown__";
  }
}

CompressRstImageZlib::CompressedImagePtr
CompressRstImageZlib::compress(const UncompressedImagePtr image) {
  auto result = CompressedImagePtr(image->New());
  result->CopyFrom(*image);

  // reserve buffer size
  ulong bound = ::compressBound(image->data().size());
  if (_BufferSize < bound) {
    _Buffer = std::unique_ptr<unsigned char>(new unsigned char[bound]);
    _BufferSize = bound;
  }

  ulong length = _BufferSize;
  auto time = boost::get_system_time();
  int error = ::compress2(_Buffer.get(), &length,
                          (const unsigned char *)image->data().c_str(),
                          image->data().size(), 1);
  std::cout << "compress: "
            << (boost::get_system_time() - time).total_nanoseconds()
            << std::endl;
  time = boost::get_system_time();

  if (error != Z_OK) {
    throw utils::Exception("Could not compress image. Error = " +
                           error_code(error));
  }
  assert(length <= std::numeric_limits<size_t>::max());

  result->set_allocated_data(
      new std::string((const char *) _Buffer.get(), (size_t)length));
  std::cout << "reduced from " << image->data().size() << " = "
            << 8 * (image->data().size() /
                    (double)(image->width() * image->height()))
            << " to " << result->data().size() << " = "
            << 8 * (result->data().size() /
                    (double)(result->width() * result->height()))
            << " = " << result->data().size() / (double)image->data().size()
            << std::endl;
  return result;
}

CompressRstImageZlib::UncompressedImagePtr
CompressRstImageZlib::decompress(const CompressedImagePtr image) {
  return image;
}
