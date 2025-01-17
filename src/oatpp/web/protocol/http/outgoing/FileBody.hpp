/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2024-present, MrBesen
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

#ifndef oatpp_web_protocol_http_outgoing_FileBody_hpp
#define oatpp_web_protocol_http_outgoing_FileBody_hpp

#include "./Body.hpp"
#include "oatpp/web/protocol/http/Http.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace outgoing {

/**
 * Implementation of &id:oatpp::web::protocol::http::outgoing::Body; class.
 * Implements functionality to use a file as data source for http body.
 */
class FileBody : public oatpp::base::Countable, public Body {
private:
  oatpp::String m_filepath;
#if defined(WIN32) || defined(_WIN32)
  oatpp::String m_buffer;
#else
  p_char8 m_ptr;
  v_int64 m_initial_size;
#endif
  oatpp::data::share::StringKeyLabel m_contentType;
  data::buffer::InlineReadData m_inlineData;
public:
  FileBody(const oatpp::String& filepath, const data::share::StringKeyLabel& contentType);
  virtual ~FileBody() override;
public:

  /**
   * Create shared FileBody.
   * @param buffer - &id:oatpp::String;.
   * @param contentType - type of the content.
   * @return - `std::shared_ptr` to FileBody.
   */
  static std::shared_ptr<FileBody> createShared(const oatpp::String& filepath,
                                                  const data::share::StringKeyLabel& contentType = data::share::StringKeyLabel());

  /**
   * Read operation callback.
   * @param buffer - pointer to buffer.
   * @param count - size of the buffer in bytes.
   * @param action - async specific action. If action is NOT &id:oatpp::async::Action::TYPE_NONE;, then
   * caller MUST return this action on coroutine iteration.
   * @return - actual number of bytes written to buffer. 0 - to indicate end-of-file.
   */
  v_io_size read(void *buffer, v_buff_size count, async::Action& action) override;

  /**
   * Declare `Content-Length` header.
   * @param headers - &id:oatpp::web::protocol::http::Headers;.
   */
  void declareHeaders(Headers& headers) override;

  /**
   * Pointer to the body known data.
   * @return - `p_char8`.
   */
  p_char8 getKnownData() override;

  /**
   * Return known size of the body.
   * @return - `v_buff_size`.
   */
  v_int64 getKnownSize() override;
  
};
  
}}}}}

#endif /* oatpp_web_protocol_http_outgoing_FileBody_hpp */
