/********************************************************************
**                                                                 **
** File   : src/io/rst/ListenerCVImage.cpp                         **
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

#include "io/rst/ListenerCVImage.h"
#include "convert/ConvertRstImageOpenCV.h"
#include "utils/CvHelpers.h"
#include <rsb/filter/TypeFilter.h>
#include <rst/vision/EncodedImage.pb.h>
#include <rst/vision/Image.pb.h>

using pontoon::io::rst::ListenerCVImageRstImage;
using pontoon::io::rst::ListenerCVImageRstEncodedImage;
using pontoon::io::rst::ListenerCVImageRstEncodedImageCollection;
using pontoon::io::rst::CombinedCVImageListener;
using pontoon::io::rst::EventData;
using rsb::filter::FilterPtr;
using rsb::filter::TypeFilter;

const std::string IPL_IMAGE_TYPE_STRING = rsc::runtime::typeName<IplImage>();
const std::string MAT_IMAGE_TYPE_STRING = rsc::runtime::typeName<cv::Mat>();

ListenerCVImageRstImage::ListenerCVImageRstImage(const std::string &uri) {
  _Listener = pontoon::utils::rsbhelpers::createListener(uri);
  _Listener->addFilter(FilterPtr(new TypeFilter(IPL_IMAGE_TYPE_STRING)));
  _Handler = boost::make_shared<rsb::EventFunctionHandler>(
      boost::bind(&ListenerCVImageRstImage::handle, this, _1));
  _Listener->addHandler(_Handler);
}

ListenerCVImageRstImage::~ListenerCVImageRstImage() {
  _Listener->removeHandler(_Handler);
}

void ListenerCVImageRstImage::handle(rsb::EventPtr data) {
  auto iplimagePtr = boost::static_pointer_cast<IplImage>(data->getData());
  rsb::EventPtr event(new rsb::Event(*data));
  event->setData(pontoon::utils::cvhelpers::asMatPtr(iplimagePtr));
  event->setType(MAT_IMAGE_TYPE_STRING);
  notify(EventData<cv::Mat>(event));
}

ListenerCVImageRstEncodedImage::ListenerCVImageRstEncodedImage(
    const std::string &uri)
    : _Listener(uri, false) {
  _Connection =
      _Listener.connect([this](EventData<::rst::vision::EncodedImage> data) {
        rsb::EventPtr event(new rsb::Event(*data.event()));
        convert::DecodeRstVisionEncodedImage decoder;
        event->setData(decoder.decode(data.data()));
        event->setType(MAT_IMAGE_TYPE_STRING);
        notify(EventData<cv::Mat>(event));
      });
}

ListenerCVImageRstEncodedImage::~ListenerCVImageRstEncodedImage() {
  _Connection.disconnect();
}

CombinedCVImageListener::CombinedCVImageListener(const std::string &uri)
    : pontoon::utils::CompositeSubject<EventData<cv::Mat>>(
          {Ptr(new ListenerCVImageRstEncodedImage(uri)),
           Ptr(new ListenerCVImageRstImage(uri))}) {}

ListenerCVImageRstEncodedImageCollection::
    ListenerCVImageRstEncodedImageCollection(const std::string &uri)
    : _Listener(uri, false) {
  _Connection = _Listener.connect(
      [this](EventData<::rst::vision::EncodedImageCollection> data) {
        rsb::EventPtr event(new rsb::Event(*data.event()));
        convert::DecodeRstVisionEncodedImage decoder;
        DataType::DataType images;
        for (auto encoded_image : data.data()->element()) {
          images.push_back(decoder.decode(encoded_image));
        }
        notify(EventDataVector<cv::Mat>(event, images));
      });
}

ListenerCVImageRstEncodedImageCollection::
    ~ListenerCVImageRstEncodedImageCollection() {
  _Connection.disconnect();
}
