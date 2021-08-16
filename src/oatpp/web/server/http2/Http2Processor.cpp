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

#include "Http2Processor.hpp"

#include "oatpp/web/protocol/http/incoming/SimpleBodyDecoder.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"

#include "oatpp/web/server/http2/Http2ProcessingComponents.hpp"

#include <arpa/inet.h>

namespace oatpp { namespace web { namespace server { namespace http2 {

const char* Http2Processor::TAG = "oatpp::web::server::http2::Http2Processor";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other

Http2Processor::ProcessingResources::ProcessingResources(const std::shared_ptr<http2::processing::Components> &pComponents,
                                                         const std::shared_ptr<oatpp::data::stream::IOStream> &pConnection)
    : components(pComponents)
    , connection(pConnection)
    , outStream(std::make_shared<http2::PriorityStreamScheduler>(connection))
    , inSettings(http2::Http2Settings::createShared())
    , outSettings(http2::Http2Settings::createShared()){
  flow = inSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE);
  hpack = protocol::http2::hpack::SimpleHpack::createShared(protocol::http2::hpack::SimpleTable::createShared(inSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE)));
  inStream = data::stream::InputStreamBufferedProxy::createShared(connection,std::make_shared<std::string>(flow,0));
}

Http2Processor::ProcessingResources::ProcessingResources(const std::shared_ptr<processing::Components> &pComponents,
                                                         const std::shared_ptr<http2::Http2Settings> &pInSettings,
                                                         const std::shared_ptr<http2::Http2Settings> &pOutSettings,
                                                         const std::shared_ptr<oatpp::data::stream::InputStreamBufferedProxy> &pInStream,
                                                         const std::shared_ptr<http2::PriorityStreamScheduler> &pOutStream)
    : components(pComponents)
    , connection(nullptr)
    , outStream(pOutStream)
    , inStream(pInStream)
    , inSettings(pInSettings)
    , outSettings(pOutSettings) {
  flow = inSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE);
  hpack = protocol::http2::hpack::SimpleHpack::createShared(protocol::http2::hpack::SimpleTable::createShared(inSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE)));
}
//
//std::shared_ptr<protocol::http::outgoing::Response>
//Http2Processor::processNextRequest(ProcessingResources &resources,
//                                   const std::shared_ptr<protocol::http::incoming::Request> &request,
//                                   ConnectionState &connectionState) {
//
//  std::shared_ptr<protocol::http::outgoing::Response> response;
//
//  try {
//
//    for (auto &interceptor : resources.components->requestInterceptors) {
//      response = interceptor->intercept(request);
//      if (response) {
//        return response;
//      }
//    }
//
//    auto route =
//        resources.components->router->getRoute(request->getStartingLine().method, request->getStartingLine().path);
//
//    if (!route) {
//
//      data::stream::BufferOutputStream ss;
//      ss << "No mapping for HTTP-method: '" << request->getStartingLine().method.toString()
//         << "', URL: '" << request->getStartingLine().path.toString() << "'";
//
//      connectionState = ConnectionState::CLOSING;
//      return resources.components->errorHandler->handleError(protocol::http::Status::CODE_404, ss.toString());
//
//    }
//
//    request->setPathVariables(route.getMatchMap());
//    return route.getEndpoint()->handle(request);
//
//  } catch (oatpp::web::protocol::http::HttpError &error) {
//    response =
//        resources.components->errorHandler->handleError(error.getInfo().status, error.getMessage(), error.getHeaders());
//    connectionState = ConnectionState::CLOSING;
//  } catch (std::exception &error) {
//    response = resources.components->errorHandler->handleError(protocol::http::Status::CODE_500, error.what());
//    connectionState = ConnectionState::CLOSING;
//  } catch (...) {
//    response = resources.components->errorHandler->handleError(protocol::http::Status::CODE_500, "Unhandled Error");
//    connectionState = ConnectionState::CLOSING;
//  }
//
//  return response;
//
//}

v_io_size Http2Processor::sendSettingsFrame(Http2Processor::ProcessingResources &resources) {
  static const Http2Settings defaultSettings;
  data::stream::BufferOutputStream bos(FrameHeader::HeaderSize+(6*6));
  bos.setCurrentPosition(3);
  FrameType type = FrameType::SETTINGS;
  bos.writeSimple(&type, 1);
  v_uint8 flags = 0;
  bos.writeSimple(&flags, 1);
  v_uint32 streamIdent = 0;
  bos.writeSimple(&streamIdent, 4);

  v_uint16 ident;
  v_uint32 setting;
  if (resources.inSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE) != defaultSettings.getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE)) {
    ident = htons(Http2Settings::SETTINGS_HEADER_TABLE_SIZE);
    setting = htonl(resources.inSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE));
    bos.writeSimple(&ident, 2);
    bos.writeSimple(&setting, 4);
  }
  if (resources.inSettings->getSetting(Http2Settings::SETTINGS_ENABLE_PUSH) != defaultSettings.getSetting(Http2Settings::SETTINGS_ENABLE_PUSH)) {
    ident = htons(Http2Settings::SETTINGS_ENABLE_PUSH);
    setting = htonl(resources.inSettings->getSetting(Http2Settings::SETTINGS_ENABLE_PUSH));
    bos.writeSimple(&ident, 2);
    bos.writeSimple(&setting, 4);
  }
  if (resources.inSettings->getSetting(Http2Settings::SETTINGS_MAX_CONCURRENT_STREAMS) != defaultSettings.getSetting(Http2Settings::SETTINGS_MAX_CONCURRENT_STREAMS)) {
    ident = htons(Http2Settings::SETTINGS_MAX_CONCURRENT_STREAMS);
    setting = htonl(resources.inSettings->getSetting(Http2Settings::SETTINGS_MAX_CONCURRENT_STREAMS));
    bos.writeSimple(&ident, 2);
    bos.writeSimple(&setting, 4);
  }
  if (resources.inSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE) != defaultSettings.getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE)) {
    ident = htons(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE);
    setting = htonl(resources.inSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE));
    bos.writeSimple(&ident, 2);
    bos.writeSimple(&setting, 4);
  }
  if (resources.inSettings->getSetting(Http2Settings::SETTINGS_MAX_FRAME_SIZE) != defaultSettings.getSetting(Http2Settings::SETTINGS_MAX_FRAME_SIZE)) {
    ident = htons(Http2Settings::SETTINGS_MAX_FRAME_SIZE);
    setting = htonl(resources.inSettings->getSetting(Http2Settings::SETTINGS_HEADER_TABLE_SIZE));
    bos.writeSimple(&ident, 2);
    bos.writeSimple(&setting, 4);
  }
  if (resources.inSettings->getSetting(Http2Settings::SETTINGS_MAX_HEADER_LIST_SIZE) != defaultSettings.getSetting(Http2Settings::SETTINGS_MAX_HEADER_LIST_SIZE)) {
    ident = htons(Http2Settings::SETTINGS_MAX_HEADER_LIST_SIZE);
    setting = htonl(resources.inSettings->getSetting(Http2Settings::SETTINGS_MAX_HEADER_LIST_SIZE));
    bos.writeSimple(&ident, 2);
    bos.writeSimple(&setting, 4);
  }
  v_uint32 payload = bos.getCurrentPosition() - 9;
  bos.setCurrentPosition(0);
  v_uint8 b =  ((payload >> 16) & 0xff);
  bos.writeSimple(&b, 1);
  b =  ((payload >> 8) & 0xff);
  bos.writeSimple(&b, 1);
  b =  ((payload) & 0xff);
  bos.writeSimple(&b, 1);
  bos.setCurrentPosition(payload + FrameHeader::HeaderSize);

  OATPP_LOGD(TAG, "Sending SETTINGS (length:%lu, flags:0x%02x, StreamId:%lu)", bos.getCurrentPosition()-FrameHeader::HeaderSize, 0, 0);
  resources.outStream->lock(PriorityStreamScheduler::PRIORITY_MAX);
  bos.flushToStream(resources.outStream.get());
  resources.outStream->unlock();
  return 9 + payload;
}

v_io_size Http2Processor::ackSettingsFrame(Http2Processor::ProcessingResources &resources) {
  FrameHeader header(0, FrameHeader::Flags::Settings::SETTINGS_ACK, FrameType::SETTINGS, 0);
  OATPP_LOGD(TAG, "Sending %s (length:%lu, flags:0x%02x, StreamId:%lu)", FrameHeader::frameTypeStringRepresentation(header.getType()), header.getLength(), header.getFlags(), header.getStreamId());
  resources.outStream->lock(PriorityStreamScheduler::PRIORITY_MAX);
  header.writeToStream(resources.outStream.get());
  resources.outStream->unlock();

  return FrameHeader::HeaderSize;
}

v_io_size Http2Processor::answerPingFrame(Http2Processor::ProcessingResources &resources) {

  data::stream::BufferOutputStream bos(FrameHeader::HeaderSize+8);
  FrameHeader header(8, FrameHeader::Flags::Ping::PING_ACK, FrameType::PING, 0);
  header.writeToStream(&bos);
  resources.inStream->readExactSizeDataSimple(bos.getData()+FrameHeader::HeaderSize, 8);
  bos.setCurrentPosition(FrameHeader::HeaderSize + 8);

  OATPP_LOGD(TAG, "Sending %s (length:%lu, flags:0x%02x, StreamId:%lu)", FrameHeader::frameTypeStringRepresentation(header.getType()), header.getLength(), header.getFlags(), header.getStreamId());
  resources.outStream->lock(PriorityStreamScheduler::PRIORITY_MAX);
  bos.flushToStream(resources.outStream.get());
  resources.outStream->unlock();

  return FrameHeader::HeaderSize+8;
}

v_io_size Http2Processor::consumeStream(const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                        v_io_size streamPayloadLength) {
  v_io_size consumed = 0;
  while (consumed < streamPayloadLength) {
    /*
     * ToDo: Discussion: We should consume the frame when its erroneous
     * But what is the best way to do that?
     */
    v_io_size chunk = std::min((v_io_size) 1024, streamPayloadLength - consumed);
    oatpp::String memory(chunk);
    data::buffer::InlineReadData inlinereader((void *) memory->data(), chunk);
    consumed += stream->readExactSizeDataSimple(inlinereader);
  }
  return consumed;
}

std::shared_ptr<Http2StreamHandler> Http2Processor::findOrCreateStream(v_uint32 ident,
                                                                       ProcessingResources &resources) {
  std::shared_ptr<Http2StreamHandler> handler;
  auto handlerentry = resources.h2streams.find(ident);
  if (handlerentry == resources.h2streams.end()) {
    handler = std::make_shared<Http2StreamHandler>(ident, resources.outStream, resources.hpack, resources.components);
    resources.h2streams.insert({ident, handler});
  } else {
    handler = handlerentry->second;
  }
  return handler;
}



Http2Processor::ConnectionState Http2Processor::processNextRequest(ProcessingResources &resources) {

  std::shared_ptr<FrameHeader> header;
  try {
    header = FrameHeader::createShared(resources.inStream);
  } catch (std::runtime_error &e) {
    return ConnectionState::DEAD;
  }

  OATPP_LOGD(TAG, "Received %s (length:%lu, flags:0x%02x, StreamId:%lu)", FrameHeader::frameTypeStringRepresentation(header->getType()), header->getLength(), header->getFlags(), header->getStreamId());

  switch (header->getType()) {

    case FrameType::PING:
      if (header->getStreamId() != 0) {
        // connection error!
        // https://datatracker.ietf.org/doc/html/rfc7540#section-5.4.1
      }
      if (header->getLength() != 8) {
        // connection error!
        // https://datatracker.ietf.org/doc/html/rfc7540#section-5.4.1
      }
      // if bit(1) (ack) is not set, reply the same payload but with flag-bit(1) set
      if (header->getFlags() == 0x00) {
        answerPingFrame(resources);
      }
      break;

    case FrameType::SETTINGS:
      if (header->getStreamId() != 0) {
        // ToDo: If an endpoint receives a SETTINGS frame whose stream identifier
        //  field is anything other than 0x0, the endpoint MUST respond with a
        //  connection error (Section 5.4.1) of type PROTOCOL_ERROR.
        //  https://datatracker.ietf.org/doc/html/rfc7540#section-6.5
      }
      if (header->getFlags() == 0) {
        for (v_uint32 consumed = 0; consumed < header->getLength(); consumed += 6) {
          v_uint16 ident;
          v_uint32 parameter;
          resources.inStream->readExactSizeDataSimple(&ident, 2);
          resources.inStream->readExactSizeDataSimple(&parameter, 4);
          try {
            resources.outSettings->setSetting((Http2Settings::Identifier) ntohs(ident), ntohl(parameter));
          } catch (std::runtime_error &e) {
            OATPP_LOGW(TAG, e.what());
          }
        }
        ackSettingsFrame(resources);
      }
      break;


    case FrameType::WINDOW_UPDATE:
      if (header->getStreamId() == 0) {

      } else {
        ConnectionState state = delegateToHandler(findOrCreateStream(header->getStreamId(), resources), resources.inStream, resources, *header);
        if (state != ConnectionState::ALIVE) {
          resources.h2streams.erase(header->getStreamId());
        }
      }
      break;

    case FrameType::DATA:
    case FrameType::HEADERS:
    case FrameType::PRIORITY:
    case FrameType::RST_STREAM:
    case FrameType::PUSH_PROMISE:
    case FrameType::GOAWAY:
    case FrameType::CONTINUATION:
      if (header->getStreamId() == 0) {
        // connection error (Section 5.4.1) of type
        //   PROTOCOL_ERROR.
        // https://datatracker.ietf.org/doc/html/rfc7540#section-5.4.1
      } else {
        ConnectionState state = delegateToHandler(findOrCreateStream(header->getStreamId(), resources), resources.inStream, resources, *header);
        if (state != ConnectionState::ALIVE) {
          resources.h2streams.erase(header->getStreamId());
        }
      }
      break;

    default:
      // ToDo: Unknown frame
      break;
  }

  return ConnectionState::ALIVE;


//  oatpp::web::protocol::http::HttpError::Info error;
//  auto headersReadResult = resources.headersReader.readHeaders(resources.inStream.get(), error);
//
//  if(error.ioStatus <= 0) {
//    return ConnectionState::DEAD;
//  }
//
//  ConnectionState connectionState = ConnectionState::ALIVE;
//  std::shared_ptr<protocol::http::incoming::Request> request;
//  std::shared_ptr<protocol::http::outgoing::Response> response;
//
//  if(error.status.code != 0) {
//    response = resources.components->errorHandler->handleError(error.status, "Invalid Request Headers");
//    connectionState = ConnectionState::CLOSING;
//  } else {
//
//    request = protocol::http::incoming::Request::createShared(resources.connection,
//                                                              headersReadResult.startingLine,
//                                                              headersReadResult.headers,
//                                                              resources.inStream,
//                                                              resources.components->bodyDecoder);
//
//    response = processNextRequest(resources, request, connectionState);
//
//    try {
//
//      for (auto& interceptor : resources.components->responseInterceptors) {
//        response = interceptor->intercept(request, response);
//        if (!response) {
//          response = resources.components->errorHandler->handleError(
//            protocol::http::Status::CODE_500,
//            "Response Interceptor returned an Invalid Response - 'null'"
//          );
//          connectionState = ConnectionState::CLOSING;
//        }
//      }
//
//    } catch (...) {
//      response = resources.components->errorHandler->handleError(
//        protocol::http::Status::CODE_500,
//        "Unhandled Error in Response Interceptor"
//      );
//      connectionState = ConnectionState::CLOSING;
//    }
//
//    response->putHeaderIfNotExists(protocol::http::Header::SERVER, protocol::http::Header::Value::SERVER);
//    protocol::http::utils::CommunicationUtils::considerConnectionState(request, response, connectionState);
//
//    switch(connectionState) {
//
//      case ConnectionState::ALIVE :
//        response->putHeaderIfNotExists(protocol::http::Header::CONNECTION, protocol::http::Header::Value::CONNECTION_KEEP_ALIVE);
//        break;
//
//      case ConnectionState::CLOSING:
//      case ConnectionState::DEAD:
//        response->putHeaderIfNotExists(protocol::http::Header::CONNECTION, protocol::http::Header::Value::CONNECTION_CLOSE);
//        break;
//
//      case ConnectionState::DELEGATED: {
//        auto handler = response->getConnectionUpgradeHandler();
//        if(handler) {
//          handler->handleConnection(resources.connection, response->getConnectionUpgradeParameters());
//          connectionState = ConnectionState::DELEGATED;
//        } else {
//          OATPP_LOGW("[oatpp::web::server::Http2Processor::processNextRequest()]", "Warning. ConnectionUpgradeHandler not set!");
//          connectionState = ConnectionState::CLOSING;
//        }
//        break;
//      }
//
//    }
//
//  }
//
//  auto contentEncoderProvider =
//    protocol::http::utils::CommunicationUtils::selectEncoder(request, resources.components->contentEncodingProviders);
//
//  response->send(resources.connection.get(), &resources.headersOutBuffer, contentEncoderProvider.get());
//
//  return connectionState;

}

Http2Processor::ConnectionState Http2Processor::delegateToHandler(const std::shared_ptr<Http2StreamHandler> &handler,
                                                                  const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                  Http2Processor::ProcessingResources &resources,
                                                                  FrameHeader &header) {
  ConnectionState state = ConnectionState::CLOSING;

  switch (header.getType()) {
    case FrameType::DATA:

      // Check if the stream is in a state where it would accept data
      // ToDo: Discussion: Should these checks be inside their respective functions?
      if (handler->getState() <= Http2StreamHandler::H2StreamState::INIT) {
        consumeStream(resources.inStream, header.getLength());
        throw std::runtime_error(
            "[oatpp::web::server::http2::Http2Processor::processNextRequest] Error: Received data for stream that has not received any headers");
      }
      if (handler->getState() >= Http2StreamHandler::H2StreamState::GOAWAY) {
        consumeStream(resources.inStream, header.getLength());
        throw std::runtime_error(
            "[oatpp::web::server::http2::Http2Processor::processNextRequest] Error: Received data for stream that is already at goaway");
      }
      state = handler->handleData(header.getFlags(), resources.inStream, header.getLength());
      break;
    case FrameType::HEADERS:
      // Check if the stream is in its initial state, i.E. the only state when headers should be acceptable
      // ToDo: Discussion: Should these checks be inside their respective functions?
      if (handler->getState() != Http2StreamHandler::H2StreamState::INIT) {
        consumeStream(resources.inStream, header.getLength());
        throw std::runtime_error(
            "[oatpp::web::server::http2::Http2Processor::processNextRequest] Error: Received headers for stream that is not in its init state");
      }
      state = handler->handleHeaders(header.getFlags(), resources.inStream, header.getLength());
      break;
    case FrameType::PRIORITY:
      state = handler->handlePriority(header.getFlags(), resources.inStream, header.getLength());
      break;
    case FrameType::RST_STREAM:
      if (handler->getState() == Http2StreamHandler::H2StreamState::INIT) {
        // ToDo: RST_STREAM frames MUST NOT be sent for a stream in the "idle" state.
        //   If a RST_STREAM frame identifying an idle stream is received, the
        //   recipient MUST treat this as a connection error (Section 5.4.1) of
        //   type PROTOCOL_ERROR.
      }
      state = handler->handleResetStream(header.getFlags(), resources.inStream, header.getLength());
      break;
    case FrameType::PUSH_PROMISE:
      state = handler->handlePushPromise(header.getFlags(), resources.inStream, header.getLength());
      break;
    case FrameType::GOAWAY:
      state = handler->handleGoAway(header.getFlags(), resources.inStream, header.getLength());
      break;
    case FrameType::WINDOW_UPDATE:
      state = handler->handleWindowUpdate(header.getFlags(), resources.inStream, header.getLength());
      break;
    case FrameType::CONTINUATION:
      // Check if the stream is in its "headers received" state, i.E. the only state when continued headers should be acceptable
      if (handler->getState() != Http2StreamHandler::H2StreamState::HEADERS) {
        consumeStream(resources.inStream, header.getLength());
        throw std::runtime_error(
            "[oatpp::web::server::http2::Http2Processor::processNextRequest] Error: Received continued headers for stream that is not in its header state");
      }
      state = handler->handleContinuation(header.getFlags(), resources.inStream, header.getLength());
      break;
    default:
      state = ConnectionState::CLOSING;
      break;
  }

  return state;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Task

Http2Processor::Task::Task(const std::shared_ptr<processing::Components> &components,
                           const std::shared_ptr<oatpp::data::stream::IOStream> &connection,
                           std::atomic_long *taskCounter)
    : m_components(components), m_connection(connection), m_counter(taskCounter), m_delegationParameters(nullptr) {
  (*m_counter)++;
}

Http2Processor::Task::Task(const std::shared_ptr<processing::Components> &components,
                           const std::shared_ptr<oatpp::data::stream::IOStream> &connection,
                           std::atomic_long *taskCounter,
                           const std::shared_ptr<const network::ConnectionHandler::ParameterMap> &delegationParameters)
    : m_components(components), m_connection(connection), m_counter(taskCounter), m_delegationParameters(delegationParameters) {
  (*m_counter)++;
}

Http2Processor::Task::Task(const Http2Processor::Task &copy)
    : m_components(copy.m_components), m_connection(copy.m_connection), m_counter(copy.m_counter) {
  (*m_counter)++;
}

Http2Processor::Task::Task(Http2Processor::Task &&move)
    : m_components(std::move(move.m_components)),
      m_connection(std::move(move.m_connection)),
      m_counter(move.m_counter) {
  move.m_counter = nullptr;
}

Http2Processor::Task &Http2Processor::Task::operator=(const Http2Processor::Task &t) {
  if (this != &t) {
    m_components = t.m_components;
    m_connection = t.m_connection;
    m_counter = t.m_counter;
    (*m_counter)++;
  }
  return *this;
}

Http2Processor::Task &Http2Processor::Task::operator=(Http2Processor::Task &&t) {
  m_components = std::move(t.m_components);
  m_connection = std::move(t.m_connection);
  m_counter = t.m_counter;
  t.m_counter = nullptr;
  return *this;
}

void Http2Processor::Task::run() {

  m_connection->initContexts();

  auto inSettings = http2::Http2Settings::createShared();
  auto outSettings = http2::Http2Settings::createShared();
  auto outStream = std::make_shared<http2::PriorityStreamScheduler>(m_connection);
  auto memlabel = std::make_shared<std::string>(inSettings->getSetting(Http2Settings::SETTINGS_INITIAL_WINDOW_SIZE), 0);
  v_io_size writePosition = 0;

  if (m_delegationParameters != nullptr) {
    auto it = m_delegationParameters->find("h2frame");
    if (it != m_delegationParameters->end()) {
      std::memcpy((void*)memlabel->data(), it->second->data(), it->second->size());
      writePosition = it->second->size() + 1;
    }
  }

  auto inStream = data::stream::InputStreamBufferedProxy::createShared(m_connection, memlabel, 0, writePosition, true);

  ProcessingResources resources(m_components, inSettings, outSettings, inStream, outStream);

  ConnectionState connectionState;

  try {

    Http2Processor::sendSettingsFrame(resources);

    do {

      connectionState = Http2Processor::processNextRequest(resources);

    } while (connectionState == ConnectionState::ALIVE);

  } catch (...) {
    // DO NOTHING
  }

}
Http2Processor::Task::~Task() {
  if (m_counter != nullptr) {
    (*m_counter)--;
  }
}

}}}}