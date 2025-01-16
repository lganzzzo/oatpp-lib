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

#include "oatpp/web/protocol/http/encoding/Chunked.hpp"
#include "oatpp/data/stream/BufferStream.hpp"
#include "oatpp/data/buffer/IOBuffer.hpp"
#include "oatpp/utils/Conversion.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace outgoing {

Request::Request(const oatpp::data::share::StringKeyLabel& method,
                 const oatpp::data::share::StringKeyLabel& path,
                 const Headers& headers,
                 const std::shared_ptr<Body>& body)
  : m_method(method)
  , m_path(path)
  , m_headers(headers)
  , m_body(body)
{}

std::shared_ptr<Request> Request::createShared(const oatpp::data::share::StringKeyLabel& method,
                                               const oatpp::data::share::StringKeyLabel& path,
                                               const Headers& headers,
                                               const std::shared_ptr<Body>& body) {
  return std::make_shared<Request>(method, path, headers, body);
}

const oatpp::data::share::StringKeyLabel& Request::getMethod() const {
  return m_method;
}

const oatpp::data::share::StringKeyLabel& Request::getPath() const {
  return m_path;
}

protocol::http::Headers& Request::getHeaders() {
  return m_headers;
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

void Request::putBundleData(const oatpp::String& key, const oatpp::Void& polymorph) {
  m_bundle.put(key, polymorph);
}

const data::Bundle& Request::getBundle() const {
  return m_bundle;
}

std::shared_ptr<Body> Request::getBody() {
  return m_body;
}

void Request::send(data::stream::OutputStream* stream){

  v_buff_size bodySize = -1;

  if(m_body){

    m_body->declareHeaders(m_headers);

    bodySize = m_body->getKnownSize();

    if(bodySize >= 0) {
      m_headers.put_LockFree(Header::CONTENT_LENGTH, utils::Conversion::int64ToStr(bodySize));
    } else {
      m_headers.put_LockFree(Header::TRANSFER_ENCODING, Header::Value::TRANSFER_ENCODING_CHUNKED);
    }

  } else {
    m_headers.put_LockFree(Header::CONTENT_LENGTH, "0");
  }

  oatpp::data::stream::BufferOutputStream buffer(2048);

  buffer.writeSimple(m_method.getData(), m_method.getSize());
  buffer.writeSimple(" /", 2);
  buffer.writeSimple(m_path.getData(), m_path.getSize());
  buffer.writeSimple(" ", 1);
  buffer.writeSimple("HTTP/1.1", 8);
  buffer.writeSimple("\r\n", 2);

  http::Utils::writeHeaders(m_headers, &buffer);

  buffer.writeSimple("\r\n", 2);

  if(m_body) {

    if(bodySize >= 0) {

      if (m_body->getKnownData() == nullptr) {
        buffer.flushToStream(stream);
        buffer.setCurrentPosition(0);
        data::stream::transfer(m_body, stream, bodySize, buffer.getData(), buffer.getCapacity());
      } else {
        if(bodySize + buffer.getCurrentPosition() < buffer.getCapacity()) {
          buffer.writeSimple(m_body->getKnownData(), bodySize);
          buffer.flushToStream(stream);
        } else {
          buffer.flushToStream(stream);
          stream->writeExactSizeDataSimple(m_body->getKnownData(), bodySize);
        }
      }

    } else {

      buffer.flushToStream(stream);

      http::encoding::EncoderChunked chunkedEncoder;

      /* Reuse headers buffer */
      buffer.setCurrentPosition(0);
      data::stream::transfer(m_body, stream, 0, buffer.getData(), buffer.getCapacity(), &chunkedEncoder);

    }

  } else {
    buffer.flushToStream(stream);
  }
  
}

oatpp::async::CoroutineStarter Request::sendAsync(std::shared_ptr<Request> _this,
                                                  const std::shared_ptr<data::stream::OutputStream>& stream)
{
  
  class SendAsyncCoroutine : public oatpp::async::Coroutine<SendAsyncCoroutine> {
  private:
    std::shared_ptr<Request> m_this;
    std::shared_ptr<data::stream::OutputStream> m_stream;
    std::shared_ptr<oatpp::data::stream::BufferOutputStream> m_headersWriteBuffer;
  public:
    
    SendAsyncCoroutine(std::shared_ptr<Request> request,
                       const std::shared_ptr<data::stream::OutputStream>& stream)
      : m_this(std::move(request))
      , m_stream(stream)
      , m_headersWriteBuffer(std::make_shared<oatpp::data::stream::BufferOutputStream>())
    {}
    
    Action act() override {

      v_buff_size bodySize = -1;

      if(m_this->m_body){

        m_this->m_body->declareHeaders(m_this->m_headers);

        bodySize = m_this->m_body->getKnownSize();

        if(bodySize >= 0) {
          m_this->m_headers.put_LockFree(Header::CONTENT_LENGTH, utils::Conversion::int64ToStr(bodySize));
        } else {
          m_this->m_headers.put_LockFree(Header::TRANSFER_ENCODING, Header::Value::TRANSFER_ENCODING_CHUNKED);
        }

      } else {
        m_this->m_headers.put_LockFree(Header::CONTENT_LENGTH, "0");
      }
      
      m_headersWriteBuffer->writeSimple(m_this->m_method.getData(), m_this->m_method.getSize());
      m_headersWriteBuffer->writeSimple(" /", 2);
      m_headersWriteBuffer->writeSimple(m_this->m_path.getData(), m_this->m_path.getSize());
      m_headersWriteBuffer->writeSimple(" ", 1);
      m_headersWriteBuffer->writeSimple("HTTP/1.1", 8);
      m_headersWriteBuffer->writeSimple("\r\n", 2);

      http::Utils::writeHeaders(m_this->m_headers, m_headersWriteBuffer.get());
      
      m_headersWriteBuffer->writeSimple("\r\n", 2);

      if(m_this->m_body) {

        if(bodySize >= 0) {

          if (m_this->m_body->getKnownData() == nullptr) {
            return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
                  .next(data::stream::transferAsync(m_this->m_body, m_stream, bodySize, data::buffer::IOBuffer::createShared()))
                  .next(finish());
          } else {
            if(bodySize + m_headersWriteBuffer->getCurrentPosition() < m_headersWriteBuffer->getCapacity()) {
              m_headersWriteBuffer->writeSimple(m_this->m_body->getKnownData(), bodySize);
              return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
                .next(finish());
            } else {
              return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
                .next(m_stream->writeExactSizeDataAsync(m_this->m_body->getKnownData(), bodySize))
                .next(finish());
            }
          }

        } else {

          auto chunkedEncoder = std::make_shared<http::encoding::EncoderChunked>();
          return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
                 .next(data::stream::transferAsync(m_this->m_body, m_stream, 0, data::buffer::IOBuffer::createShared(), chunkedEncoder))
                 .next(finish());

        }

      } else {

        return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
          .next(finish());
      }
      
    }
    
  };
  
  return SendAsyncCoroutine::start(std::move(_this), stream);
  
}
  
}}}}}
