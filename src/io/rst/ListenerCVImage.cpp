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

#include <io/rst/ListenerCVImage.h>

using io::rst::ListenerCVImage;

ListenerCVImage::ListenerCVImage(const std::string& scope)
{
  m_Listener = utils::rsbhelpers::createListener(scope);
  m_Handler = boost::make_shared<rsb::EventFunctionHandler>(boost::bind(&ListenerCVImage::handle,this,_1));
  m_Listener->addHandler(m_Handler);
}

ListenerCVImage::~ListenerCVImage(){
  m_Listener->removeHandler(m_Handler);
}

void ListenerCVImage::handle(rsb::EventPtr data){
  this->notify(boost::static_pointer_cast<DataType>(data->getData()));
}
