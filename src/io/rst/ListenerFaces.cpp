/********************************************************************
**                                                                 **
** File   : src/io/rst/ListenerFaces.cpp                           **
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

#include "io/rst/ListenerFaces.h"
#include "io/rst/Listener.h"
#include <rst/vision/FaceWithGazeCollection.pb.h>
#include <rst/vision/Faces.pb.h>

using pontoon::io::rst::ListenerFacesRst;
using ::rst::vision::Face;
using ::rst::vision::Faces;
using ::rst::vision::FaceWithGazeCollection;

ListenerFacesRst::ListenerFacesRst(const std::string &uri) {
  pontoon::utils::rsbhelpers::register_rst<Faces, FaceWithGazeCollection>();
  _listener = pontoon::utils::rsbhelpers::createListener(uri);
  _handler = boost::make_shared<rsb::EventFunctionHandler>([this](
      rsb::EventPtr event) {
    DataType::Data faces;
    if (auto event_data = EventData<Faces>(event)) {
      for (const auto &face : event_data.data()->faces()) {
        faces.push_back(boost::make_shared<Face>(face));
      }
    } else if (auto event_data = EventData<FaceWithGazeCollection>(event)) {
      for (const auto &face_with_gaze : event_data.data()->element()) {
        if (face_with_gaze.has_region()) {
          faces.push_back(boost::make_shared<Face>(face_with_gaze.region()));
        }
      }
    }
    this->notify(DataType(event, faces));
  });
  _listener->addHandler(_handler);
}

ListenerFacesRst::~ListenerFacesRst() { _listener->removeHandler(_handler); }
