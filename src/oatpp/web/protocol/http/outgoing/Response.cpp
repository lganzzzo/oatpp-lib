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

#include "./Response.hpp"

#include "oatpp/web/protocol/http/encoding/Chunked.hpp"
#include "oatpp/utils/Conversion.hpp"

namespace oatpp { namespace web { namespace protocol { namespace http { namespace outgoing {

Response::Response(const Status& status,
                   const std::shared_ptr<Body>& body)
  : m_status(status)
  , m_body(body)
{}

std::shared_ptr<Response> Response::createShared(const Status& status,
                                                 const std::shared_ptr<Body>& body) {
  return std::make_shared<Response>(status, body);
}

const Status& Response::getStatus() const {
  return m_status;
}

protocol::http::Headers& Response::getHeaders() {
  return m_headers;
}

std::shared_ptr<Body> Response::getBody() const {
  return m_body;
}

void Response::putHeader(const oatpp::String& key, const oatpp::String& value) {
  m_headers.put(key, value);
}

bool Response::putHeaderIfNotExists(const oatpp::String& key, const oatpp::String& value) {
  return m_headers.putIfNotExists(key, value);
}

bool Response::putOrReplaceHeader(const String &key, const String &value) {
  return m_headers.putOrReplace(key, value);
}

bool Response::putOrReplaceHeader_Unsafe(const data::share::StringKeyLabelCI& key,
                                         const data::share::StringKeyLabel &value) {
  return m_headers.putOrReplace(key, value);
}

void Response::putHeader_Unsafe(const oatpp::data::share::StringKeyLabelCI& key, const oatpp::data::share::StringKeyLabel& value) {
  m_headers.put(key, value);
}

bool Response::putHeaderIfNotExists_Unsafe(const oatpp::data::share::StringKeyLabelCI& key, const oatpp::data::share::StringKeyLabel& value) {
  return m_headers.putIfNotExists(key, value);
}

oatpp::String Response::getHeader(const oatpp::data::share::StringKeyLabelCI& headerName) const {
  return m_headers.get(headerName);
}

void Response::putBundleData(const oatpp::String& key, const oatpp::Void& polymorph) {
  m_bundle.put(key, polymorph);
}

const data::Bundle& Response::getBundle() const {
  return m_bundle;
}

void Response::setConnectionUpgradeHandler(const std::shared_ptr<oatpp::network::ConnectionHandler>& handler) {
  m_connectionUpgradeHandler = handler;
}

std::shared_ptr<oatpp::network::ConnectionHandler> Response::getConnectionUpgradeHandler() {
  return m_connectionUpgradeHandler;
}

void Response::setConnectionUpgradeParameters(const std::shared_ptr<const ConnectionHandler::ParameterMap>& parameters) {
  m_connectionUpgradeParameters = parameters;
}

std::shared_ptr<const Response::ConnectionHandler::ParameterMap> Response::getConnectionUpgradeParameters() {
  return m_connectionUpgradeParameters;
}

void Response::send(data::stream::OutputStream* stream,
                    data::stream::BufferOutputStream* headersWriteBuffer,
                    http::encoding::EncoderProvider* contentEncoderProvider)
{

  v_int64 bodySize = -1;

  if(m_body){

    m_body->declareHeaders(m_headers);

    if(contentEncoderProvider == nullptr) {

      bodySize = m_body->getKnownSize();

      if (bodySize >= 0) {
        m_headers.put_LockFree(Header::CONTENT_LENGTH, utils::Conversion::int64ToStr(bodySize));
      } else {
        m_headers.put_LockFree(Header::TRANSFER_ENCODING, Header::Value::TRANSFER_ENCODING_CHUNKED);
      }

    } else {
      m_headers.put_LockFree(Header::TRANSFER_ENCODING, Header::Value::TRANSFER_ENCODING_CHUNKED);
      m_headers.put_LockFree(Header::CONTENT_ENCODING, contentEncoderProvider->getEncodingName());
    }

  } else {
    m_headers.put_LockFree(Header::CONTENT_LENGTH, "0");
  }

  headersWriteBuffer->setCurrentPosition(0);

  headersWriteBuffer->writeSimple("HTTP/1.1 ", 9);
  headersWriteBuffer->writeAsString(m_status.code);
  headersWriteBuffer->writeSimple(" ", 1);
  headersWriteBuffer->writeSimple(m_status.description);
  headersWriteBuffer->writeSimple("\r\n", 2);

  http::Utils::writeHeaders(m_headers, headersWriteBuffer);

  headersWriteBuffer->writeSimple("\r\n", 2);

  if(m_body) {

    if(contentEncoderProvider == nullptr) {

      if (bodySize >= 0) {

        if(m_body->getKnownData() == nullptr) {
          headersWriteBuffer->flushToStream(stream);
          /* Reuse headers buffer */
          /* Transfer without chunked encoder */
          data::stream::transfer(m_body, stream, 0, headersWriteBuffer->getData(), headersWriteBuffer->getCapacity());
        } else { 
          if (bodySize + headersWriteBuffer->getCurrentPosition() < headersWriteBuffer->getCapacity()) {
            headersWriteBuffer->writeSimple(m_body->getKnownData(), bodySize);
            headersWriteBuffer->flushToStream(stream);
          } else {
            headersWriteBuffer->flushToStream(stream);
            stream->writeExactSizeDataSimple(m_body->getKnownData(), bodySize);
          }
        }
      } else {

        headersWriteBuffer->flushToStream(stream);

        http::encoding::EncoderChunked chunkedEncoder;

        /* Reuse headers buffer */
        data::stream::transfer(m_body, stream, 0, headersWriteBuffer->getData(), headersWriteBuffer->getCapacity(), &chunkedEncoder);

      }

    } else {

      headersWriteBuffer->flushToStream(stream);

      http::encoding::EncoderChunked chunkedEncoder;
      auto contentEncoder = contentEncoderProvider->getProcessor();

      data::buffer::ProcessingPipeline pipeline({
        contentEncoder,
        &chunkedEncoder
      });

      /* Reuse headers buffer */
      data::stream::transfer(m_body, stream, 0, headersWriteBuffer->getData(), headersWriteBuffer->getCapacity(), &pipeline);

    }

  } else {
    headersWriteBuffer->flushToStream(stream);
  }

}

oatpp::async::CoroutineStarter Response::sendAsync(const std::shared_ptr<Response>& _this,
                                                   const std::shared_ptr<data::stream::OutputStream>& stream,
                                                   const std::shared_ptr<oatpp::data::stream::BufferOutputStream>& headersWriteBuffer,
                                                   const std::shared_ptr<http::encoding::EncoderProvider>& contentEncoder)
{

  class SendAsyncCoroutine : public oatpp::async::Coroutine<SendAsyncCoroutine> {
  private:
    std::shared_ptr<Response> m_this;
    std::shared_ptr<data::stream::OutputStream> m_stream;
    std::shared_ptr<oatpp::data::stream::BufferOutputStream> m_headersWriteBuffer;
    std::shared_ptr<http::encoding::EncoderProvider> m_contentEncoderProvider;
  public:

    SendAsyncCoroutine(const std::shared_ptr<Response>& _this,
                       const std::shared_ptr<data::stream::OutputStream>& stream,
                       const std::shared_ptr<oatpp::data::stream::BufferOutputStream>& headersWriteBuffer,
                       const std::shared_ptr<http::encoding::EncoderProvider>& contentEncoderProvider)
      : m_this(_this)
      , m_stream(stream)
      , m_headersWriteBuffer(headersWriteBuffer)
      , m_contentEncoderProvider(contentEncoderProvider)
    {}

    Action act() override {

      v_buff_size bodySize = -1;

      if(m_this->m_body){

        m_this->m_body->declareHeaders(m_this->m_headers);

        if(!m_contentEncoderProvider) {

          bodySize = m_this->m_body->getKnownSize();

          if (bodySize >= 0) {
            m_this->m_headers.put_LockFree(Header::CONTENT_LENGTH, utils::Conversion::int64ToStr(bodySize));
          } else {
            m_this->m_headers.put_LockFree(Header::TRANSFER_ENCODING, Header::Value::TRANSFER_ENCODING_CHUNKED);
          }

        } else {
          m_this->m_headers.put_LockFree(Header::TRANSFER_ENCODING, Header::Value::TRANSFER_ENCODING_CHUNKED);
          m_this->m_headers.put_LockFree(Header::CONTENT_ENCODING, m_contentEncoderProvider->getEncodingName());
        }

      } else {
        m_this->m_headers.put_LockFree(Header::CONTENT_LENGTH, "0");
      }

      m_headersWriteBuffer->setCurrentPosition(0);

      m_headersWriteBuffer->writeSimple("HTTP/1.1 ", 9);
      m_headersWriteBuffer->writeAsString(m_this->m_status.code);
      m_headersWriteBuffer->writeSimple(" ", 1);
      m_headersWriteBuffer->writeSimple(m_this->m_status.description);
      m_headersWriteBuffer->writeSimple("\r\n", 2);

      http::Utils::writeHeaders(m_this->m_headers, m_headersWriteBuffer.get());

      m_headersWriteBuffer->writeSimple("\r\n", 2);

      if(m_this->m_body) {

        if(!m_contentEncoderProvider) {

          if (bodySize >= 0) {

            if(m_this->m_body->getKnownData() == nullptr) {
              return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
                .next(data::stream::transferAsync(m_this->m_body, m_stream, bodySize, data::buffer::IOBuffer::createShared()))
                .next(finish());
            } else {
              if (bodySize + m_headersWriteBuffer->getCurrentPosition() < m_headersWriteBuffer->getCapacity()) {

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

          auto chunkedEncoder = std::make_shared<http::encoding::EncoderChunked>();
          auto contentEncoder = m_contentEncoderProvider->getProcessor();

          auto pipeline = std::shared_ptr<data::buffer::ProcessingPipeline>(new data::buffer::ProcessingPipeline({
            contentEncoder,
            chunkedEncoder
          }));

          return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
            .next(data::stream::transferAsync(m_this->m_body, m_stream, 0, data::buffer::IOBuffer::createShared(), pipeline)
            . next(finish()));

        }

      } else {

        return oatpp::data::stream::BufferOutputStream::flushToStreamAsync(m_headersWriteBuffer, m_stream)
          .next(finish());
      }

    }

  };

  return SendAsyncCoroutine::start(_this, stream, headersWriteBuffer, contentEncoder);

}

}}}}}
