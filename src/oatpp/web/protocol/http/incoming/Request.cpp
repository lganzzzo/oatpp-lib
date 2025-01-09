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

#include "Request.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace incoming {

Request::Request(const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
                 const http::RequestStartingLine& startingLine,
                 const http::Headers& headers,
                 const std::shared_ptr<oatpp::data::stream::InputStream>& bodyStream,
                 const std::shared_ptr<const http::incoming::BodyDecoder>& bodyDecoder)
  : m_connection(connection)
  , m_startingLine(startingLine)
  , m_headers(headers)
  , m_bodyStream(bodyStream)
  , m_bodyDecoder(bodyDecoder)
  , m_queryParamsParsed(false)
  , m_bodyRead(false)
{}

std::shared_ptr<Request> Request::createShared(const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
                                               const http::RequestStartingLine& startingLine,
                                               const http::Headers& headers,
                                               const std::shared_ptr<oatpp::data::stream::InputStream>& bodyStream,
                                               const std::shared_ptr<const http::incoming::BodyDecoder>& bodyDecoder)
{
  return std::make_shared<Request>(connection, startingLine, headers, bodyStream, bodyDecoder);
}

std::shared_ptr<oatpp::data::stream::IOStream> Request::getConnection() {
  return m_connection;
}

const http::RequestStartingLine& Request::getStartingLine() const {
  return m_startingLine;
}

void Request::setPathVariables(const url::mapping::Pattern::MatchMap& pathVariables) {
  m_pathVariables = pathVariables;
}

const url::mapping::Pattern::MatchMap& Request::getPathVariables() const {
  return m_pathVariables;
}

const http::Headers& Request::getHeaders() const {
  return m_headers;
}

const http::QueryParams& Request::getQueryParameters() const {
  if(!m_queryParamsParsed) {
    oatpp::network::Url::Parser::parseQueryParams(m_queryParams, m_pathVariables.getTail());
    m_queryParamsParsed = true;
  }
  return m_queryParams;
}

oatpp::String Request::getQueryParameter(const oatpp::data::share::StringKeyLabel& name) const {
  return getQueryParameters().get(name);
}

oatpp::String Request::getQueryParameter(const oatpp::data::share::StringKeyLabel& name, const oatpp::String& defaultValue) const {
  auto value = getQueryParameter(name);
  return value ? value : defaultValue;
}

std::shared_ptr<oatpp::data::stream::InputStream> Request::getBodyStream() const {
  return m_bodyStream;
}

std::shared_ptr<const http::incoming::BodyDecoder> Request::getBodyDecoder() const {
  return m_bodyDecoder;
}

bool Request::hasReadBody() const {
  return m_bodyRead;
}

void Request::putHeader(const oatpp::String& key, const oatpp::String& value) {
  m_headers.put(key, value);
}

bool Request::putHeaderIfNotExists(const oatpp::String& key, const oatpp::String& value) {
  return m_headers.putIfNotExists(key, value);
}

bool Request::putOrReplaceHeader(const String &key, const String &value) {
  return m_headers.putOrReplace(key, value);
}

bool Request::putOrReplaceHeader_Unsafe(const data::share::StringKeyLabelCI& key,
                                        const data::share::StringKeyLabel &value) {
  return m_headers.putOrReplace(key, value);
}

void Request::putHeader_Unsafe(const oatpp::data::share::StringKeyLabelCI& key, const oatpp::data::share::StringKeyLabel& value) {
  m_headers.put(key, value);
}

bool Request::putHeaderIfNotExists_Unsafe(const oatpp::data::share::StringKeyLabelCI& key, const oatpp::data::share::StringKeyLabel& value) {
  return m_headers.putIfNotExists(key, value);
}

oatpp::String Request::getHeader(const oatpp::data::share::StringKeyLabelCI& headerName) const{
  return m_headers.get(headerName);
}

std::vector<oatpp::String> Request::getHeaderValues(const oatpp::data::share::StringKeyLabelCI& headerName) const {
  std::vector<oatpp::String> result;
  auto range = m_headers.getAll_Unsafe().equal_range(headerName);
  for (auto it = range.first; it != range.second; ++it) {
    result.emplace_back(it->second.toString());
  }
  return result;
}

oatpp::String Request::getPathVariable(const oatpp::data::share::StringKeyLabel& name) const {
  return m_pathVariables.getVariable(name);
}

oatpp::String Request::getPathTail() const {
  return m_pathVariables.getTail();
}

void Request::putBundleData(const oatpp::String& key, const oatpp::Void& polymorph) {
  m_bundle.put(key, polymorph);
}

const data::Bundle& Request::getBundle() const {
  return m_bundle;
}

void Request::transferBody(const base::ObjectHandle<data::stream::WriteCallback>& writeCallback) const {
  m_bodyRead = true;
  m_bodyDecoder->decode(m_headers, m_bodyStream.get(), writeCallback.get(), m_connection.get());
}

void Request::transferBodyToStream(const base::ObjectHandle<oatpp::data::stream::OutputStream>& toStream) const {
  m_bodyRead = true;
  m_bodyDecoder->decode(m_headers, m_bodyStream.get(), toStream.get(), m_connection.get());
}

oatpp::String Request::readBodyToString() const {
  m_bodyRead = true;
  return m_bodyDecoder->decodeToString(m_headers, m_bodyStream.get(), m_connection.get());
}

async::CoroutineStarter Request::transferBodyAsync(const std::shared_ptr<data::stream::WriteCallback>& writeCallback) const {
  m_bodyRead = true;
  return m_bodyDecoder->decodeAsync(m_headers, m_bodyStream, writeCallback, m_connection);
}

async::CoroutineStarter Request::transferBodyToStreamAsync(const std::shared_ptr<oatpp::data::stream::OutputStream>& toStream) const {
  m_bodyRead = true;
  return m_bodyDecoder->decodeAsync(m_headers, m_bodyStream, toStream, m_connection);
}

async::CoroutineStarterForResult<const oatpp::String&> Request::readBodyToStringAsync() const {
  m_bodyRead = true;
  return m_bodyDecoder->decodeToStringAsync(m_headers, m_bodyStream, m_connection);
}

}}}}}
