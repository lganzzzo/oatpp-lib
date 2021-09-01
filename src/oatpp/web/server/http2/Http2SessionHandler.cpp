/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Benedikt-Alexander Mokroß <github@bamkrs.de>
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

#include "Http2SessionHandler.hpp"

#include "oatpp/core/data/stream/BufferStream.hpp"

#include "oatpp/web/server/http2/Http2ProcessingComponents.hpp"

#include "oatpp/core/async/worker/IOWorker.hpp"

#include <arpa/inet.h>

namespace oatpp { namespace web { namespace server { namespace http2 {

const char* Http2SessionHandler::TAG = "oatpp::web::server::http2::Http2SessionHandler";
const Http2Settings Http2SessionHandler::DEFAULT_SETTINGS;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other

Http2SessionHandler::ProcessingResources::ProcessingResources(const std::shared_ptr<processing::Components> &pComponents,
                                                         const std::shared_ptr<http2::Http2Settings> &pInSettings,
                                                         const std::shared_ptr<http2::Http2Settings> &pOutSettings,
                                                         const std::shared_ptr<oatpp::data::stream::InputStreamBufferedProxy> &pInStream,
                                                         const std::shared_ptr<http2::PriorityStreamSchedulerAsync> &pOutStream)
    : components(pComponents)
    , connection(nullptr)
    , outStream(pOutStream)
    , inStream(pInStream)
    , inSettings(pInSettings)
    , outSettings(pOutSettings)
    , h2streams()
    , lastStream(nullptr)
    , highestNonIdleStreamId(0) {
  hpack = protocol::http2::hpack::SimpleHpack::createShared(protocol::http2::hpack::SimpleTable::createShared(inSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE)), inSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE));
//  OATPP_LOGD("oatpp::web::server::http2::Http2SessionHandler::ProcessingResources", "Constructing %p", this);
}

Http2SessionHandler::ProcessingResources::~ProcessingResources() {
//  OATPP_LOGD("oatpp::web::server::http2::Http2SessionHandler::ProcessingResources", "Destructing %p", this);
}

//
//std::shared_ptr<protocol::http::outgoing::Response>
//Http2SessionHandler::processNextRequest(ProcessingResources &m_resources,
//                                   const std::shared_ptr<protocol::http::incoming::Request> &request,
//                                   ConnectionState &connectionState) {
//
//  std::shared_ptr<protocol::http::outgoing::Response> response;
//
//  try {
//
//    for (auto &interceptor : m_resources->components->requestInterceptors) {
//      response = interceptor->intercept(request);
//      if (response) {
//        return response;
//      }
//    }
//
//    auto route =
//        m_resources->components->router->getRoute(request->getStartingLine().method, request->getStartingLine().path);
//
//    if (!route) {
//
//      data::stream::BufferOutputStream ss;
//      ss << "No mapping for HTTP-method: '" << request->getStartingLine().method.toString()
//         << "', URL: '" << request->getStartingLine().path.toString() << "'";
//
//      connectionState = ConnectionState::CLOSING;
//      return m_resources->components->errorHandler->handleError(protocol::http::Status::CODE_404, ss.toString());
//
//    }
//
//    request->setPathVariables(route.getMatchMap());
//    return route.getEndpoint()->handle(request);
//
//  } catch (oatpp::web::protocol::http::HttpError &error) {
//    response =
//        m_resources->components->errorHandler->handleError(error.getInfo().status, error.getMessage(), error.getHeaders());
//    connectionState = ConnectionState::CLOSING;
//  } catch (std::exception &error) {
//    response = m_resources->components->errorHandler->handleError(protocol::http::Status::CODE_500, error.what());
//    connectionState = ConnectionState::CLOSING;
//  } catch (...) {
//    response = m_resources->components->errorHandler->handleError(protocol::http::Status::CODE_500, "Unhandled Error");
//    connectionState = ConnectionState::CLOSING;
//  }
//
//  return response;
//
//}

async::Action Http2SessionHandler::sendSettingsFrame(async::Action &&next) {

  class SendSettingsFrameCoroutine : public SendFrameCoroutine<SendSettingsFrameCoroutine> {
   private:
    const std::shared_ptr<Http2Settings> m_settings;
    v_uint8 m_data[6*6];
    data::share::MemoryLabel m_label;

   public:
    SendSettingsFrameCoroutine(const std::shared_ptr<PriorityStreamSchedulerAsync> &output, const std::shared_ptr<Http2Settings> &settings)
      : SendFrameCoroutine<SendSettingsFrameCoroutine>(output, FrameType::SETTINGS, 0, 0, PriorityStreamSchedulerAsync::PRIORITY_MAX)
      , m_settings(settings)
      , m_label(nullptr, m_data, 6*6) {
      static const Http2Settings::Identifier idents[] = {
          Http2Settings::SETTINGS_HEADER_TABLE_SIZE,
          Http2Settings::SETTINGS_ENABLE_PUSH,
          Http2Settings::SETTINGS_MAX_CONCURRENT_STREAMS,
          Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE,
          Http2Settings::SETTINGS_MAX_FRAME_SIZE,
          Http2Settings::SETTINGS_MAX_HEADER_LIST_SIZE
      };
      p_uint8 dataptr = m_data;
      v_buff_size count = 0;

      for (int i = 0; i < 6; ++i) {
        if (settings->getSetting(idents[i]) != DEFAULT_SETTINGS.getSetting(idents[i])) {
          auto setting = settings->getSetting(idents[i]);
          *dataptr++ = (idents[i] >> 8) & 0xff;
          *dataptr++ = idents[i] & 0xff;
          *dataptr++ = (setting >> 24) & 0xff;
          *dataptr++ = (setting >> 16) & 0xff;
          *dataptr++ = (setting >> 8) & 0xff;
          *dataptr++ = setting & 0xff;
          count += 6;
        }
      }
      m_label = data::share::MemoryLabel(nullptr, m_data, count);
    }
  };

  return SendSettingsFrameCoroutine::start(m_resources->outStream, m_resources->inSettings).next(std::forward<async::Action>(next));
}

async::Action Http2SessionHandler::ackSettingsFrame() {
  class SendAckSettingsFrameCoroutine : public SendFrameCoroutine<SendAckSettingsFrameCoroutine> {
   public:
    SendAckSettingsFrameCoroutine(const std::shared_ptr<PriorityStreamSchedulerAsync> &output)
      : SendFrameCoroutine<SendAckSettingsFrameCoroutine>(output, FrameType::SETTINGS, 0, FrameHeader::Flags::Settings::SETTINGS_ACK, PriorityStreamSchedulerAsync::PRIORITY_MAX)
      {}
  };
  return SendAckSettingsFrameCoroutine::start(m_resources->outStream).next(yieldTo(&Http2SessionHandler::nextRequest));
}

async::Action Http2SessionHandler::answerPingFrame() {
  std::shared_ptr<std::string> handle = std::make_shared<std::string>(8, (char)0);

  class SendPingFrameCoroutine : public SendFrameCoroutine<SendPingFrameCoroutine> {
   private:
    data::share::MemoryLabel m_label;
   public:
    const data::share::MemoryLabel *frameData() const override {
      return &m_label;
    }

    SendPingFrameCoroutine(const std::shared_ptr<PriorityStreamSchedulerAsync> &output, const data::share::MemoryLabel &label)
      : SendFrameCoroutine<SendPingFrameCoroutine>(output, FrameType::PING, 0, FrameHeader::Flags::Ping::PING_ACK, PriorityStreamSchedulerAsync::PRIORITY_MAX)
      , m_label(label) {}
  };

  data::buffer::InlineReadData readData((void*)handle->data(), handle->size());
  return m_resources->inStream->readSomeDataAsyncInline(
      readData,
      SendPingFrameCoroutine::start(m_resources->outStream, handle).next(yieldTo(&Http2SessionHandler::nextRequest))
      );
}

async::Action Http2SessionHandler::sendGoawayFrame(v_uint32 lastStream, protocol::http2::error::ErrorCode errorCode) {
 class SendGoawayFrameCoroutine : public SendFrameCoroutine<SendGoawayFrameCoroutine> {
  private:
   data::share::MemoryLabel m_label;
   v_uint8 buf[8];

  public:
   SendGoawayFrameCoroutine(const std::shared_ptr<PriorityStreamSchedulerAsync> &output, v_uint32 lastStream, H2ErrorCode errorCode)
     : SendFrameCoroutine<SendGoawayFrameCoroutine>(output, FrameType::GOAWAY, 0, 0, PriorityStreamSchedulerAsync::PRIORITY_MAX){
     p_uint8 data = buf;
     *data++ = (lastStream >> 24) & 0xff;
     *data++ = (lastStream >> 16) & 0xff;
     *data++ = (lastStream >> 8) & 0xff;
     *data++ = lastStream & 0xff;
     *data++ = (errorCode >> 24) & 0xff;
     *data++ = (errorCode >> 16) & 0xff;
     *data++ = (errorCode >> 8) & 0xff;
     *data++ = errorCode & 0xff;
   }

   const data::share::MemoryLabel *frameData() const override {
     return &m_label;
   }
 };

  return SendGoawayFrameCoroutine::start(m_resources->outStream, lastStream, errorCode).next(yieldTo(&Http2SessionHandler::teardown));
}

async::Action Http2SessionHandler::sendResetStreamFrame(v_uint32 stream, protocol::http2::error::ErrorCode errorCode) {
  class SendResetStreamFrameCoroutine : public SendFrameCoroutine<SendResetStreamFrameCoroutine> {
   private:
    std::shared_ptr<ProcessingResources> m_resources;
    data::share::MemoryLabel m_label;
    v_uint32 m_errorCode;

   public:
    SendResetStreamFrameCoroutine(const std::shared_ptr<PriorityStreamSchedulerAsync> &output, v_uint32 stream, H2ErrorCode errorCode)
        : SendFrameCoroutine<SendResetStreamFrameCoroutine>(output, FrameType::RST_STREAM, stream, 0, PriorityStreamSchedulerAsync::PRIORITY_MAX)
        , m_errorCode(htonl(errorCode))
        , m_label(nullptr, &m_errorCode, 4) {}

    const data::share::MemoryLabel *frameData() const override {
      return &m_label;
    }
  };

  return SendResetStreamFrameCoroutine::start(m_resources->outStream, stream, errorCode).next(yieldTo(&Http2SessionHandler::nextRequest));
}

async::Action Http2SessionHandler::consumeStream(v_io_size streamPayloadLength, async::Action &&action) {
  class StreamConsumerCoroutine : public async::Coroutine<StreamConsumerCoroutine> {
   private:
    std::shared_ptr<data::stream::InputStream> m_stream;
    v_buff_size m_consume;
    data::buffer::IOBuffer m_buffer;

   public:
    StreamConsumerCoroutine(const std::shared_ptr<data::stream::InputStream> &stream, v_buff_size consume)
      : m_stream(stream)
      , m_consume(consume) {}

    Action act() override {
      if (m_consume > 0) {
        v_io_size chunk = std::min((v_io_size) m_buffer.getSize(), m_consume);
        data::buffer::InlineReadData inlinereader((void *) m_buffer.getData(), chunk);
        m_consume -= chunk;
        return m_stream->readSomeDataAsyncInline(inlinereader, yieldTo(&StreamConsumerCoroutine::act));
      }
      return finish();
    }
  };
  return StreamConsumerCoroutine::start(m_resources->inStream, streamPayloadLength).next(std::forward<async::Action>(action));
}

std::shared_ptr<Http2StreamHandler::Task> Http2SessionHandler::findOrCreateStream(v_uint32 ident) {
  std::shared_ptr<Http2StreamHandler::Task> handler;
  auto handlerentry = m_resources->h2streams.find(ident);
  if (handlerentry == m_resources->h2streams.end()) {
    if (ident < m_resources->highestNonIdleStreamId) {
      throw protocol::http2::error::connection::ProtocolError("[oatpp::web::server::http2::Http2SessionHandler::findOrCreateStream] Error: Tried to create a new stream with a streamId smaller than the currently highest known streamId");
    }
    handler = std::make_shared<Http2StreamHandler::Task>(ident, m_resources->outStream, m_resources->hpack, m_resources->components, m_resources->inSettings, m_resources->outSettings);
    m_resources->h2streams.insert({ident, handler});
  } else {
    handler = handlerentry->second;
  }
  return handler;
}

async::Action Http2SessionHandler::nextRequest() {
  return FrameHeader::readFrameHeaderAsync(m_resources->inStream).callbackTo(&Http2SessionHandler::handleFrame);
}

async::Action Http2SessionHandler::handleFrame(const std::shared_ptr<FrameHeader> &header) {

  OATPP_LOGD(TAG, "Received %s (length:%lu, flags:0x%02x, streamId:%lu)", FrameHeader::frameTypeStringRepresentation(header->getType()), header->getLength(), header->getFlags(), header->getStreamId());

  // streamId's have to be uneven numbers
  if ((header->getStreamId() != 0) && (header->getStreamId() & 0x01) == 0) {
    return connectionError(protocol::http2::error::ErrorCode::PROTOCOL_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received even streamId (PROTOCOL_ERROR)");
  }

  if (m_resources->lastStream && (header->getStreamId() != m_resources->lastStream->streamId)) {
    // Headers have to be sent as continous frames of HEADERS, PUSH_PROMISE and CONTINUATION.
    // Only if the last opened stream transitioned beyond its HEADERS stage, other frames are allowed
    if (m_resources->lastStream->state < Http2StreamHandler::H2StreamState::PAYLOAD && m_resources->lastStream->state > Http2StreamHandler::H2StreamState::INIT) {
      return connectionError(protocol::http2::error::ErrorCode::PROTOCOL_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Last opened stream is still in its HEADER state but frame for different stream received (PROTOCOL_ERROR)");
    }
  }

  try {
    switch (header->getType()) {

      case FrameType::PING:
        if (header->getStreamId() != 0) {
          return connectionError(H2ErrorCode::PROTOCOL_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received PING frame on stream.");
        }
        if (header->getLength() != 8) {
          return connectionError(H2ErrorCode::FRAME_SIZE_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received PING with invalid frame size.");
        }
        // if bit(1) (ack) is not set, reply the same payload but with flag-bit(1) set
        if ((header->getFlags() & 0x01) == 0x00) {
          return answerPingFrame();
        } else if ((header->getFlags() & 0x01) == 0x01) {
          // all good, just consume
          consumeStream(8, yieldTo(&Http2SessionHandler::nextRequest));
        } else {
          // unknown flags! for now, just consume
          consumeStream(8, yieldTo(&Http2SessionHandler::nextRequest));
        }
        break;

      case FrameType::SETTINGS:
        if (header->getStreamId() != 0) {
          return connectionError(H2ErrorCode::PROTOCOL_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received SETTINGS frame on stream.");
        }
        if (header->getFlags() == 0) {
          if (header->getLength() % 6) {
            return connectionError(H2ErrorCode::FRAME_SIZE_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received SETTINGS with invalid frame size.");
          }
          v_uint32 initialWindowSize = m_resources->outSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE);
          v_uint32 tableSize = m_resources->outSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE);
          for (v_uint32 consumed = 0; consumed < header->getLength(); consumed += 6) {
            v_uint16 ident;
            v_uint32 parameter;
            m_resources->inStream->readExactSizeDataSimple(&ident, 2);
            m_resources->inStream->readExactSizeDataSimple(&parameter, 4);
            ident = ntohs(ident);
            try {
              m_resources->outSettings->setSetting((Http2Settings::Identifier) ident, ntohl(parameter));
            } catch (protocol::http2::error::connection::ProtocolError &h2pe) {
              return connectionError(H2ErrorCode::PROTOCOL_ERROR, h2pe.what());
            } catch (protocol::http2::error::connection::FlowControlError &h2fce) {
              return connectionError(H2ErrorCode::FLOW_CONTROL_ERROR, h2fce.what());
            } catch (std::runtime_error &e) {
              OATPP_LOGW(TAG, e.what());
            }
          }
          v_uint32 newInitialWindowSize = m_resources->outSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE);
          v_uint32 newTableSize = m_resources->outSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE);
          if (initialWindowSize != newInitialWindowSize) {
            v_int32 change = 0;
            if(initialWindowSize > newInitialWindowSize) {
              change -= (initialWindowSize - newInitialWindowSize);
            } else {
              change = newInitialWindowSize - initialWindowSize;
            }
            for (auto &h2s : m_resources->h2streams) {
              h2s.second->resizeWindow(change);
            }
          }
          if (tableSize != newTableSize) {
            m_resources->hpack->setMaxTableSize(newTableSize);
          }
          return ackSettingsFrame();
        } else if (header->getFlags() == 0x01 && header->getLength() > 0) {
          return connectionError(H2ErrorCode::FRAME_SIZE_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received SETTINGS ack with payload.");
        }
        break;

      case FrameType::GOAWAY:
        if (header->getStreamId() == 0){
          v_uint32 errorCode, lastId, highest = 0;
          m_resources->inStream->readExactSizeDataSimple(&lastId, 4);
          m_resources->inStream->readExactSizeDataSimple(&errorCode, 4);
          errorCode = ntohl(errorCode);
          lastId = ntohl(lastId);
          // Consume remaining debug data.
          if (header->getLength() > 8) {
            v_uint32 remaining = header->getLength() - 8;
            while (remaining > 0) {
              v_uint32 chunk = std::min((v_uint32) 2048, remaining);
              v_char8 buf[chunk];
              m_resources->inStream->readExactSizeDataSimple(buf, chunk);
              remaining -= chunk;
            }
          }
          // ToDo: Handle additional frames, then stop
          // m_resources->inStream->setInputStreamIOMode(data::stream::ASYNCHRONOUS);
          // while (processNextRequest(m_resources) != ConnectionState::DEAD) {}
          return yieldTo(&Http2SessionHandler::teardown);
        } else {
          return connectionError(H2ErrorCode::PROTOCOL_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received GOAWAY on stream.");
        }

      case FrameType::WINDOW_UPDATE:
        if (header->getStreamId() == 0) {
          if (header->getLength() != 4) {
            return connectionError(H2ErrorCode::FRAME_SIZE_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received WINDOW_UPDATE with invalid frame size");
          }
          v_uint32 increment;
          m_resources->inStream->readExactSizeDataSimple(&increment, 4);
          increment = ntohl(increment);
          if (increment == 0) {
            return connectionError(H2ErrorCode::PROTOCOL_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Increment of 0");
          }
          OATPP_LOGD(TAG, "Incrementing out-window by %u", increment);
          m_resources->outSettings->setSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE, m_resources->outSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE)+increment);
        } else {
          return delegateToHandler(findOrCreateStream(header->getStreamId()), *header);
        }
        break;

      case FrameType::DATA:
      case FrameType::HEADERS:
      case FrameType::PRIORITY:
      case FrameType::RST_STREAM:
      case FrameType::PUSH_PROMISE:
      case FrameType::CONTINUATION:
        if (header->getStreamId() == 0) {
          throw protocol::http2::error::connection::ProtocolError("[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received stream related frame on stream(0)");
        } else {
          m_resources->lastStream = findOrCreateStream(header->getStreamId());
          return delegateToHandler(m_resources->lastStream, *header);
        }
        break;

      default:
        // ToDo: Unknown frame
        return consumeStream(header->getLength(), yieldTo(&Http2SessionHandler::nextRequest));
    }
  } catch (protocol::http2::error::Error &h2e) {
    OATPP_LOGE(TAG, "%s (%s)", h2e.what(), h2e.getH2ErrorCodeString());
    if (h2e.getH2ErrorScope() == protocol::http2::error::CONNECTION) {
      return sendGoawayFrame(m_resources->lastStream ? m_resources->lastStream->streamId : 0, h2e.getH2ErrorCode());
    } else {
      return sendResetStreamFrame(header->getStreamId(), h2e.getH2ErrorCode());
    }
  }

  return repeat();
}


async::Action Http2SessionHandler::stop() {
  return finish();
}

async::Action Http2SessionHandler::delegateToHandler(const std::shared_ptr<Http2StreamHandler::Task> &handlerTask,
                                                     FrameHeader &header) {
  Http2StreamHandler::H2StreamState state = Http2StreamHandler::H2StreamState::INIT;

  switch (header.getType()) {
    case FrameType::DATA:

      // Check if the stream is in a state where it would accept data
      // ToDo: Discussion: Should these checks be inside their respective functions?
      if (handlerTask->state != Http2StreamHandler::H2StreamState::PAYLOAD) {
        if (handlerTask->state >= Http2StreamHandler::H2StreamState::PROCESSING) {
          return connectionError(protocol::http2::error::STREAM_CLOSED,
              "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received data for stream that is already (half-)closed");
        }
        return connectionError(protocol::http2::error::PROTOCOL_ERROR,
            "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received data for stream that is not in payload state");
      }
      state = Http2StreamHandler::handleData(handlerTask, header.getFlags(), m_resources->inStream, header.getLength());
      break;
    case FrameType::HEADERS:
      if (handlerTask->state >= Http2StreamHandler::H2StreamState::PROCESSING) {
        throw protocol::http2::error::connection::StreamClosed(
            "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received headers for stream that is already (half-)closed");
      }
      if (handlerTask->state > m_resources->highestNonIdleStreamId) {
        m_resources->highestNonIdleStreamId = handlerTask->streamId;
      }
      state = Http2StreamHandler::handleHeaders(handlerTask, header.getFlags(), m_resources->inStream, header.getLength());
      break;
    case FrameType::PRIORITY:
      if (handlerTask->state == Http2StreamHandler::H2StreamState::HEADERS || handlerTask->state == Http2StreamHandler::H2StreamState::CONTINUATION) {
        throw protocol::http2::error::connection::ProtocolError("[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received PRIORITY frame while still in header state.");
      }
      state = Http2StreamHandler::handlePriority(handlerTask, header.getFlags(), m_resources->inStream, header.getLength());
      break;
    case FrameType::RST_STREAM:
      if (handlerTask->state == Http2StreamHandler::H2StreamState::INIT) {
        throw protocol::http2::error::connection::ProtocolError("[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received RST_STREAM on an idle stream.");
      }
      state = Http2StreamHandler::handleResetStream(handlerTask, header.getFlags(), m_resources->inStream, header.getLength());
      return sendResetStreamFrame(handlerTask->streamId, protocol::http2::error::ErrorCode::CANCEL);
      break;
    case FrameType::PUSH_PROMISE:
      state = Http2StreamHandler::handlePushPromise(handlerTask, header.getFlags(), m_resources->inStream, header.getLength());
      break;
    case FrameType::WINDOW_UPDATE:
      if (handlerTask->state == Http2StreamHandler::H2StreamState::INIT) {
        throw protocol::http2::error::connection::ProtocolError("[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received WINDOW_UPDATE on an idle stream.");
      }
      state = Http2StreamHandler::handleWindowUpdate(handlerTask, header.getFlags(), m_resources->inStream, header.getLength());
      break;
    case FrameType::CONTINUATION:
      // Check if the stream is in its "headers received" state, i.E. the only state when continued headers should be acceptable
      if (handlerTask->state != Http2StreamHandler::H2StreamState::HEADERS && handlerTask->state != Http2StreamHandler::H2StreamState::CONTINUATION) {
        return connectionError(H2ErrorCode::PROTOCOL_ERROR, "[oatpp::web::server::http2::Http2SessionHandler::processNextRequest] Error: Received continued headers for stream that is not in its header state");
      }
      state = Http2StreamHandler::handleContinuation(handlerTask, header.getFlags(), m_resources->inStream, header.getLength());
      break;
  }

  if (state == Http2StreamHandler::H2StreamState::READY) {
    m_executor->execute<Http2StreamHandler>(handlerTask);
  }
  return yieldTo(&Http2SessionHandler::nextRequest);
}

async::Action Http2SessionHandler::connectionError(Http2SessionHandler::H2ErrorCode errorCode) {
  return sendGoawayFrame(m_resources->lastStream ? m_resources->lastStream->streamId : 0, errorCode);
}

async::Action Http2SessionHandler::connectionError(Http2SessionHandler::H2ErrorCode errorCode,
                                                   const std::string &message) {
  OATPP_LOGE(TAG, "%s: %s", protocol::http2::error::stringRepresentation(errorCode), message.data());
  return connectionError(errorCode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup

Http2SessionHandler::Http2SessionHandler(const std::shared_ptr<processing::Components> &components,
                           const std::shared_ptr<oatpp::data::stream::IOStream> &connection,
                           std::atomic_long *taskCounter,
                           oatpp::async::Executor *executor)
    : Http2SessionHandler(components, connection, taskCounter, executor, nullptr) {}

Http2SessionHandler::Http2SessionHandler(const std::shared_ptr<processing::Components> &components,
                           const std::shared_ptr<oatpp::data::stream::IOStream> &connection,
                           std::atomic_long *sessionCounter,
                           oatpp::async::Executor *executor,
                           const std::shared_ptr<const network::ConnectionHandler::ParameterMap> &delegationParameters)
    : m_components(components), m_connection(connection), m_counter(sessionCounter), m_executor(executor) {
  (*m_counter)++;
  m_connection->initContexts();

  auto inSettings = http2::Http2Settings::createShared();
  auto outSettings = http2::Http2Settings::createShared();
  auto outStream = std::make_shared<http2::PriorityStreamSchedulerAsync>(m_connection);
  auto memlabel = std::make_shared<std::string>(inSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE), 0);
  v_io_size writePosition = 0;

  if (delegationParameters != nullptr) {
    auto it = delegationParameters->find("h2frame");
    if (it != delegationParameters->end()) {
      std::memcpy((void*)memlabel->data(), it->second->data(), it->second->size());
      writePosition = it->second->size() + 1;
    }
  }

  auto inStream = data::stream::InputStreamBufferedProxy::createShared(m_connection, memlabel, 0, writePosition, true);

  m_resources = ProcessingResources::createShared(m_components, inSettings, outSettings, inStream, outStream);
}

Http2SessionHandler::Http2SessionHandler(const Http2SessionHandler &copy)
    : m_components(copy.m_components), m_connection(copy.m_connection), m_counter(copy.m_counter), m_resources(copy.m_resources), m_executor(copy.m_executor) {
  (*m_counter)++;
}

Http2SessionHandler::Http2SessionHandler(Http2SessionHandler &&move)
    : m_components(std::move(move.m_components))
    , m_connection(std::move(move.m_connection))
    , m_counter(move.m_counter)
    , m_resources(std::move(move.m_resources))
    , m_executor(std::move(move.m_executor)){
  move.m_counter = nullptr;
  move.m_executor = nullptr;
}

Http2SessionHandler &Http2SessionHandler::operator=(const Http2SessionHandler &t) {
  if (this != &t) {
    m_components = t.m_components;
    m_connection = t.m_connection;
    m_counter = t.m_counter;
    m_resources = t.m_resources;
    m_executor = t.m_executor;
    (*m_counter)++;
  }
  return *this;
}

Http2SessionHandler &Http2SessionHandler::operator=(Http2SessionHandler &&t) {
  m_components = std::move(t.m_components);
  m_connection = std::move(t.m_connection);
  m_resources = std::move(t.m_resources);
  m_executor = t.m_executor;
  t.m_executor = nullptr;
  m_counter = t.m_counter;
  t.m_counter = nullptr;
  return *this;
}

async::Action Http2SessionHandler::act() {
  try {
    return Http2SessionHandler::sendSettingsFrame(yieldTo(&Http2SessionHandler::nextRequest));
  } catch (...) {
    return finish();
  }
}

async::Action Http2SessionHandler::teardown() {
  return finish();
}

Http2SessionHandler::~Http2SessionHandler() noexcept {
  if (m_counter != nullptr) {
    (*m_counter)--;
  }
}
async::Action Http2SessionHandler::handleError(async::Error *error) {
  OATPP_LOGE(TAG, error->what());
  return AbstractCoroutine::handleError(error);
}

}}}}