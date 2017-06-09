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

#include <convert/ConvertRstImageOpenCV.h>
#include <io/rst/ListenerCVImage.h>
#include <rsb/filter/TypeFilter.h>
#include <rst/vision/EncodedImage.pb.h>
#include <rst/vision/Image.pb.h>

using pontoon::io::rst::ListenerCVImageRstImage;
using pontoon::io::rst::ListenerCVImageRstEncodedImage;
using pontoon::io::rst::CombinedCVImageListener;
using pontoon::io::rst::EventData;
using rsb::filter::FilterPtr;
using rsb::filter::TypeFilter;

const std::string IMAGE_TYPE_STRING = rsc::runtime::typeName<IplImage>();

ListenerCVImageRstImage::ListenerCVImageRstImage(const std::string &uri) {
  m_Listener = pontoon::utils::rsbhelpers::createListener(uri);
  m_Listener->addFilter(FilterPtr(new TypeFilter(IMAGE_TYPE_STRING)));
  m_Handler = boost::make_shared<rsb::EventFunctionHandler>(
      boost::bind(&ListenerCVImageRstImage::handle, this, _1));
  m_Listener->addHandler(m_Handler);
}

ListenerCVImageRstImage::~ListenerCVImageRstImage() {
  m_Listener->removeHandler(m_Handler);
}

void ListenerCVImageRstImage::handle(rsb::EventPtr data) {
  this->notify(EventData<IplImage>(data));
}

ListenerCVImageRstEncodedImage::ListenerCVImageRstEncodedImage(
    const std::string &uri)
    : m_Listener(uri, false) {
  m_Connection = m_Listener.connect([this](EventData<::rst::vision::EncodedImage> data) {
      rsb::EventPtr event(new rsb::Event(*data.event()));
      convert::DecodeRstVisionEncodedImage decoder;
      event->setData(decoder.decode(data.data()));
      event->setType(IMAGE_TYPE_STRING);
      notify(EventData<IplImage>(event));
  });
}

ListenerCVImageRstEncodedImage::~ListenerCVImageRstEncodedImage() {
  m_Connection.disconnect();
}

CombinedCVImageListener::CombinedCVImageListener(const std::string &uri)
    : pontoon::utils::CompositeSubject<EventData<IplImage>>(
          {Ptr(new ListenerCVImageRstEncodedImage(uri)),
           Ptr(new ListenerCVImageRstImage(uri))}) {}
