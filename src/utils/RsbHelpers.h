/********************************************************************
**                                                                 **
** File   : src/utils/RsbHelpers.h                                 **
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

#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>
#include <rsb/Scope.h>
#include <rsb/ParticipantConfig.h>
#include <rsb/Factory.h>

namespace utils {
namespace rsbhelpers {

template<typename Type>
void register_rst(){
  // try to register converter if not already done
  try {
    boost::shared_ptr<rsb::converter::ProtocolBufferConverter<Type> > converter(
      new rsb::converter::ProtocolBufferConverter<Type>());
    rsb::converter::converterRepository<std::string>()->registerConverter(converter);
  } catch (const std::exception& e) {
    // already available do nothing
  }
}

template<typename First, typename Second, typename... Rest>
void register_rst(){
  // register list recursive
  register_rst<First>();
  register_rst<Second, Rest...>();
}

template<typename VectorType, typename OptionalData>
void optional_push_back(VectorType& vec, const OptionalData& data){
  if(data){
    vec.push_back(data.get());
  }
}

std::tuple<rsb::Scope,rsb::ParticipantConfig> parseUri(const std::string& uri,
                                                      rsb::ParticipantConfig config
                                                      = rsb::getFactory().getDefaultParticipantConfig());

inline rsb::ListenerPtr createListener(const std::string& uri,
                                       const rsb::ParticipantConfig& config
                                         = rsb::getFactory().getDefaultParticipantConfig(),
                                       rsb::ParticipantPtr parent = rsb::ParticipantPtr())
{
  auto parsed = parseUri(uri,config);
  return rsb::getFactory().createListener(std::get<0>(parsed),std::get<1>(parsed),parent);
}

template<class DataType>
typename rsb::Informer<DataType>::Ptr createInformer(const std::string& uri, const rsb::ParticipantConfig& config
                                                       = rsb::getFactory().getDefaultParticipantConfig(),
                                                     const std::string& dataType = rsb::detail::TypeName<DataType>()(),
                                                     rsb::ParticipantPtr parent = rsb::ParticipantPtr())
{
    auto parsed = parseUri(uri, config);
    return rsb::getFactory().createInformer<DataType>(std::get<0>(parsed),std::get<1>(parsed),dataType,parent);
}

} // namespace utils
} // namespace rsbhelpers
