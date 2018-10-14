/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "./Primitive.hpp"

#include "oatpp/core/utils/ConversionUtils.hpp"

namespace oatpp { namespace data { namespace mapping { namespace type {

  const char* const __class::String::CLASS_NAME_NONCONSTEXPR = "String";
  const char* const __class::Int8::CLASS_NAME_NONCONSTEXPR = "Int8";
  const char* const __class::Int16::CLASS_NAME_NONCONSTEXPR = "Int16";
  const char* const __class::Int32::CLASS_NAME_NONCONSTEXPR = "Int32";
  const char* const __class::Int64::CLASS_NAME_NONCONSTEXPR = "Int64";
  const char* const __class::Float32::CLASS_NAME_NONCONSTEXPR = "Float32";
  const char* const __class::Float64::CLASS_NAME_NONCONSTEXPR = "Float64";
  const char* const __class::Boolean::CLASS_NAME_NONCONSTEXPR = "Boolean";

String::String(const std::shared_ptr<oatpp::base::StrBuffer>& ptr, const type::Type* const valueType)
  : oatpp::data::mapping::type::ObjectWrapper<oatpp::base::StrBuffer, __class::String>(ptr)
{
  if(type::__class::String::getType() != valueType) {
    throw std::runtime_error("Value type does not match");
  }
}
  
String operator + (const char* a, const String& b) {
  return oatpp::base::StrBuffer::createSharedConcatenated(a, (v_int32) std::strlen(a), b->getData(), b->getSize());
}

String operator + (const String& b, const char* a) {
  return oatpp::base::StrBuffer::createSharedConcatenated(b->getData(), b->getSize(), a, (v_int32) std::strlen(a));
}

String operator + (const String& a, const String& b) {
  return oatpp::base::StrBuffer::createSharedConcatenated(a->getData(), a->getSize(), b->getData(), b->getSize());
}
  
namespace __class {
  
  type::Int8 Int8::parseFromString(const oatpp::String& str, bool& success) {
    return utils::conversion::strToInt32(str, success);
  }
  
  type::Int16 Int16::parseFromString(const oatpp::String& str, bool& success) {
    return utils::conversion::strToInt32(str, success);
  }
  
  type::Int32 Int32::parseFromString(const oatpp::String& str, bool& success) {
    return utils::conversion::strToInt32(str, success);
  }
  
  type::Int64 Int64::parseFromString(const oatpp::String& str, bool& success) {
    return utils::conversion::strToInt64(str, success);
  }
  
  type::Float32 Float32::parseFromString(const oatpp::String& str, bool& success) {
    return utils::conversion::strToFloat32(str, success);
  }
  
  type::Float64 Float64::parseFromString(const oatpp::String& str, bool& success) {
    return utils::conversion::strToFloat64(str, success);
  }
  
  type::Boolean Boolean::parseFromString(const oatpp::String& str, bool& success) {
    return utils::conversion::strToBool(str, success);
  }
  
}
  
}}}}
