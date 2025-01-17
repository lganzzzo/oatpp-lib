/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
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

#ifndef oatpp_web_protocol_http_outgoing_ResponseFactory_hpp
#define oatpp_web_protocol_http_outgoing_ResponseFactory_hpp

#include "./Response.hpp"

#include "oatpp/data/mapping/ObjectMapper.hpp"
#include "oatpp/data/type/Type.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace outgoing {

/**
 * Helper class to create http outgoing responses (&id:oatpp::web::protocol::http::outgoing::Response).
 */
class ResponseFactory {
public:
  
  /**
   * Create &id:oatpp::web::protocol::http::outgoing::Response; without a &id:oatpp::web::protocol::http::outgoing::Body;.
   * @param status - &id:oatpp::web::protocol::http::Status;.
   * @return - `std::shared_ptr` to &id:oatpp::web::protocol::http::outgoing::Response;.
   */
  static std::shared_ptr<Response> createResponse(const Status &status);

  /**
   * Create &id:oatpp::web::protocol::http::outgoing::Response; with &id:oatpp::web::protocol::http::outgoing::BufferBody;.
   * @param status - &id:oatpp::web::protocol::http::Status;.
   * @param text - &id:oatpp::String;.
   * @return - `std::shared_ptr` to &id:oatpp::web::protocol::http::outgoing::Response;.
   */
  static std::shared_ptr<Response> createResponse(const Status& status, const oatpp::String& text);

  /**
   * Create &id:oatpp::web::protocol::http::outgoing::Response; with &id:oatpp::web::protocol::http::outgoing::FileBody;.
   * @param status - &id:oatpp::web::protocol::http::Status;.
   * @param filepath - &id:oatpp::String;.
   * @return - `std::shared_ptr` to &id:oatpp::web::protocol::http::outgoing::Response;.
   */
  static std::shared_ptr<Response> createFileResponse(const Status& status, const oatpp::String& filepath);

  /**
   * Create &id:oatpp::web::protocol::http::outgoing::Response; with &id:oatpp::web::protocol::http::outgoing::DtoBody;.
   * @param status - &id:oatpp::web::protocol::http::Status;.
   * @param dto - see [Data Transfer Object (DTO)](https://oatpp.io/docs/components/dto/).
   * @param objectMapper - &id:oatpp::data::mapping::ObjectMapper;.
   * @return - `std::shared_ptr` to &id:oatpp::web::protocol::http::outgoing::Response;.
   */
  static std::shared_ptr<Response> createResponse(const Status& status,
                                                  const oatpp::Void& dto,
                                                  const std::shared_ptr<data::mapping::ObjectMapper>& objectMapper);
  
};
  
}}}}}

#endif /* oatpp_web_protocol_http_outgoing_ResponseFactory_hpp */
