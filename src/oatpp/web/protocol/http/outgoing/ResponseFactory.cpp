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

#include "./ResponseFactory.hpp"

#include "./BufferBody.hpp"
#include "./FileBody.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace outgoing {

std::shared_ptr<Response>
ResponseFactory::createResponse(const Status& status) {
  return Response::createShared(status, nullptr);
}

std::shared_ptr<Response>
ResponseFactory::createResponse(const Status& status, const oatpp::String& text) {
  return Response::createShared(status, BufferBody::createShared(text));
}

std::shared_ptr<Response>
ResponseFactory::createFileResponse(const Status& status, const oatpp::String& filepath) {
  return Response::createShared(status, FileBody::createShared(filepath));
}

std::shared_ptr<Response>
ResponseFactory::createResponse(const Status& status,
                                const oatpp::Void& dto,
                                const std::shared_ptr<data::mapping::ObjectMapper>& objectMapper) {
  return Response::createShared(status, BufferBody::createShared(
    objectMapper->writeToString(dto),
    objectMapper->getInfo().httpContentType
  ));
}

  
}}}}}
