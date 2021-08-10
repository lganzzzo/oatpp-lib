/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *                         Benedikt-Alexander Mokroß <github@bamkrs.de>
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

#ifndef oatpp_web_server_http2_ConnectionHandler_hpp
#define oatpp_web_server_http2_ConnectionHandler_hpp

#include "oatpp/web/server/HttpConnectionHandler.hpp"

namespace oatpp { namespace web { namespace server { namespace http2 {

 class ConnectionHandler : public base::Countable, public network::ConnectionHandler {

 public:
  /**
   * Implementation of &id:oatpp::network::ConnectionHandler::handleConnection;.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   */
  void handleConnection(const std::shared_ptr<IOStream>& connection, const std::shared_ptr<const ParameterMap>& params) override;


};

}}}}

#endif //oatpp_web_server_http2_ConnectionHandler_hpp
