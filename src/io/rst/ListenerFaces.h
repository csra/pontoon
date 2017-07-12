/********************************************************************
**                                                                 **
** File   : src/io/rst/ListenerFaces.h                             **
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

#include "io/rst/Listener.h"
#include "utils/RsbHelpers.h"
#include "utils/Subject.h"
#include <boost/make_shared.hpp>
#include <rst/vision/Face.pb.h>

namespace pontoon {
namespace io {
namespace rst {

class ListenerFacesRst
    : public pontoon::utils::Subject<EventDataVector<::rst::vision::Face>> {
public:
  ListenerFacesRst(const std::string &uri);

  ~ListenerFacesRst();

private:
  rsb::ListenerPtr _listener;
  rsb::HandlerPtr _handler;
};

} // namespace rst
} // namespace io
} // namespace pontoon
