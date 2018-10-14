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

#ifndef oatpp_web_server_handler_ErrorHandler_hpp
#define oatpp_web_server_handler_ErrorHandler_hpp

#include "oatpp/web/protocol/http/outgoing/Response.hpp"
#include "oatpp/web/protocol/http/Http.hpp"

namespace oatpp { namespace web { namespace server { namespace handler {
  
class ErrorHandler {
public:
  
  virtual
  std::shared_ptr<protocol::http::outgoing::Response>
  handleError(const protocol::http::Status& status, const oatpp::String& message) = 0;
  
};
  
class DefaultErrorHandler : public oatpp::base::Controllable, public ErrorHandler {
public:
  DefaultErrorHandler()
  {}
public:
  
  static std::shared_ptr<DefaultErrorHandler> createShared() {
    return std::make_shared<DefaultErrorHandler>();
  }
  
  std::shared_ptr<protocol::http::outgoing::Response>
  handleError(const protocol::http::Status& status, const oatpp::String& message) override;
  
};
  
}}}}

#endif /* oatpp_web_server_handler_ErrorHandler_hpp */
