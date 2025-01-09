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

#ifndef oatpp_web_protocol_http_incoming_Request_hpp
#define oatpp_web_protocol_http_incoming_Request_hpp

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/web/protocol/http/incoming/BodyDecoder.hpp"
#include "oatpp/web/url/mapping/Pattern.hpp"
#include "oatpp/network/Url.hpp"
#include "oatpp/data/Bundle.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace incoming {

/**
 * Class http::incoming::Request AKA IncomingRequest represents client's incoming request.
 */
class Request : public oatpp::base::Countable {
private:

  std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
  http::RequestStartingLine m_startingLine;
  url::mapping::Pattern::MatchMap m_pathVariables;
  http::Headers m_headers;
  std::shared_ptr<oatpp::data::stream::InputStream> m_bodyStream;
  
  /*
   * Request should be preconfigured with default BodyDecoder.
   * Custom BodyDecoder can be set on demand
   */
  std::shared_ptr<const http::incoming::BodyDecoder> m_bodyDecoder;

  mutable bool m_queryParamsParsed; // used for lazy parsing of QueryParams
  mutable bool m_bodyRead;
  mutable http::QueryParams m_queryParams;

  data::Bundle m_bundle;

public:
  
  Request(const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
          const http::RequestStartingLine& startingLine,
          const http::Headers& headers,
          const std::shared_ptr<oatpp::data::stream::InputStream>& bodyStream,
          const std::shared_ptr<const http::incoming::BodyDecoder>& bodyDecoder);
public:
  
  static std::shared_ptr<Request> createShared(const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
                                               const http::RequestStartingLine& startingLine,
                                               const http::Headers& headers,
                                               const std::shared_ptr<oatpp::data::stream::InputStream>& bodyStream,
                                               const std::shared_ptr<const http::incoming::BodyDecoder>& bodyDecoder);

  /**
   * Get raw connection stream.
   * @return - &id:std::shared_ptr<oatpp::data::stream::IOStream> m_connection;.
   */
  std::shared_ptr<oatpp::data::stream::IOStream> getConnection();

  /**
   * Get map of url query parameters.
   * Query parameters will be lazy parsed from url "tail"
   * Please note: lazy parsing of query parameters is not thread-safe!
   * @return map<key, value> for "&key=value"
   */
  const http::QueryParams& getQueryParameters() const;

  /**
   * Get query parameter value by name
   * @param name
   * @return query parameter value
   */
  oatpp::String getQueryParameter(const oatpp::data::share::StringKeyLabel& name) const;

  /**
   * Get query parameter value by name with defaultValue
   * @param name
   * @param defaultValue
   * @return query parameter value or defaultValue if no such parameter found
   */
  oatpp::String getQueryParameter(const oatpp::data::share::StringKeyLabel& name, const oatpp::String& defaultValue) const;

  /**
   * Get request starting line. (method, path, protocol)
   * @return starting line structure
   */
  const http::RequestStartingLine& getStartingLine() const;

  /**
   * Set request path variables.
   * @param pathVariables - &id:oatpp::web::url::mapping::Pattern::MatchMap;.
   */
  void setPathVariables(const url::mapping::Pattern::MatchMap& pathVariables);

  /**
   * Get path variables according to path-pattern. <br>
   * Ex. given request path="/sum/19/1" for path-pattern="/sum/{a}/{b}" <br>
   * getPathVariables().getVariable("a") == 19, getPathVariables().getVariable("b") == 1.
   *
   * @return url MatchMap
   */
  const url::mapping::Pattern::MatchMap& getPathVariables() const;

  /**
   * Get request's headers map
   * @return Headers map
   */
  const http::Headers& getHeaders() const;

  /**
   * Get request's body stream
   * @return body stream
   */
  std::shared_ptr<oatpp::data::stream::InputStream> getBodyStream() const;

  /**
   * Get body decoder.
   * @return Body decoder
   */
  std::shared_ptr<const http::incoming::BodyDecoder> getBodyDecoder() const;

  /**
   * Check if body was read.
   * @return - `true` if body was read.
   */
  bool hasReadBody() const;

  /**
   * Add http header.
   * @param key - &id:oatpp::String;.
   * @param value - &id:oatpp::String;.
   */
  void putHeader(const oatpp::String& key, const oatpp::String& value);

  /**
   * Add http header if not already exists.
   * @param key - &id:oatpp::String;.
   * @param value - &id:oatpp::String;.
   * @return - `true` if header was added.
   */
  bool putHeaderIfNotExists(const oatpp::String& key, const oatpp::String& value);

  /**
   * Replaces or adds header.
   * @param key - &id:oatpp::String;.
   * @param value - &id:oatpp::String;.
   * @return - `true` if header was replaces, `false` if header was added.
   */
  bool putOrReplaceHeader(const oatpp::String& key, const oatpp::String& value);

  /**
   * Replaces or adds header.
   * @param key - &id:oatpp::data::share::StringKeyLabelCI;.
   * @param value - &id:oatpp::data::share::StringKeyLabel;.
   * @return - `true` if header was replaces, `false` if header was added.
   */
  bool putOrReplaceHeader_Unsafe(const oatpp::data::share::StringKeyLabelCI& key, const oatpp::data::share::StringKeyLabel& value);

  /**
   * Add http header.
   * @param key - &id:oatpp::data::share::StringKeyLabelCI;.
   * @param value - &id:oatpp::data::share::StringKeyLabel;.
   */
  void putHeader_Unsafe(const oatpp::data::share::StringKeyLabelCI& key, const oatpp::data::share::StringKeyLabel& value);

  /**
   * Add http header if not already exists.
   * @param key - &id:oatpp::data::share::StringKeyLabelCI;.
   * @param value - &id:oatpp::data::share::StringKeyLabel;.
   * @return - `true` if header was added.
   */
  bool putHeaderIfNotExists_Unsafe(const oatpp::data::share::StringKeyLabelCI& key, const oatpp::data::share::StringKeyLabel& value);

  /**
   * Get header value
   * @param headerName - &id:oatpp::data::share::StringKeyLabelCI;.
   * @return - &id:oatpp::String;.
   */
  oatpp::String getHeader(const oatpp::data::share::StringKeyLabelCI& headerName) const;

  /**
   * Get all header values
   * @param headerName
   * @return
   */
  std::vector<oatpp::String> getHeaderValues(const oatpp::data::share::StringKeyLabelCI& headerName) const;

  /**
   * Get path variable according to path-pattern
   * @param name
   * @return matched value for path-pattern
   */
  oatpp::String getPathVariable(const oatpp::data::share::StringKeyLabel& name) const;

  /**
   * Get path tail according to path-pattern
   * Ex. given request path="/hello/path/tail" for path-pattern="/hello/\*"
   * tail == "path/tail"
   * note '/' symbol is required before '*'
   * @return matched tail-value for path-pattern
   */
  oatpp::String getPathTail() const;

  /**
   * Put data to bundle.
   * @param key
   * @param polymorph
   */
  void putBundleData(const oatpp::String& key, const oatpp::Void& polymorph);

  /**
   * Get data from bundle by key.
   * @tparam WrapperType
   * @param key
   * @return
   */
  template<typename WrapperType>
  WrapperType getBundleData(const oatpp::String& key) const {
    return m_bundle.template get<WrapperType>(key);
  }

  /**
   * Get bundle object.
   * @return
   */
  const data::Bundle& getBundle() const;

  /**
   * Transfer body. <br>
   * Read body chunk by chunk and pass chunks to the `writeCallback`.
   * @param writeCallback - &id:oatpp::data::stream::WriteCallback;.
   */
  void transferBody(const base::ObjectHandle<data::stream::WriteCallback>& writeCallback) const;

  /**
   * Stream content of the body-stream to toStream
   * @param toStream
   */
  void transferBodyToStream(const base::ObjectHandle<data::stream::OutputStream>& toStream) const;

  /**
   * Transfer body stream to string
   * @return body as string
   */
  oatpp::String readBodyToString() const;

  /**
   * Transfer body to String and parse it as DTO
   * @tparam Wrapper - ObjectWrapper type.
   * @param objectMapper
   * @return DTO
   */
  template<class Wrapper>
  Wrapper readBodyToDto(const base::ObjectHandle<data::mapping::ObjectMapper>& objectMapper) const {
    m_bodyRead = true;
    return objectMapper->readFromString<Wrapper>(m_bodyDecoder->decodeToString(m_headers, m_bodyStream.get(), m_connection.get()));
  }
  
  // Async

  /**
   * Transfer body in Asynchronous manner. <br>
   * Read body chunk by chunk and pass chunks to the `writeCallback`.
   * @param writeCallback - `std::shared_ptr` to &id:oatpp::data::stream::WriteCallback;.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  async::CoroutineStarter transferBodyAsync(const std::shared_ptr<data::stream::WriteCallback>& writeCallback) const;

  /**
   * Transfer body stream to toStream Async
   * @param toStream
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  oatpp::async::CoroutineStarter transferBodyToStreamAsync(const std::shared_ptr<oatpp::data::stream::OutputStream>& toStream) const;

  /**
   * Transfer body stream to string Async.
   * @return - &id:oatpp::async::CoroutineStarterForResult;.
   */
  async::CoroutineStarterForResult<const oatpp::String&> readBodyToStringAsync() const;

  /**
   * Transfer body to String and parse it as DTO
   * @tparam Wrapper - DTO `ObjectWrapper`.
   * @param objectMapper
   * @return - &id:oatpp::async::CoroutineStarterForResult;.
   */
  template<class Wrapper>
  oatpp::async::CoroutineStarterForResult<const Wrapper&>
  readBodyToDtoAsync(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper) const {
    m_bodyRead = true;
    return m_bodyDecoder->decodeToDtoAsync<Wrapper>(m_headers, m_bodyStream, m_connection, objectMapper);
  }
  
};
  
}}}}}

#endif /* oatpp_web_protocol_http_incoming_Request_hpp */

