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

#include "HttpProcessor.hpp"

#include "oatpp/web/server/HttpServerError.hpp"
#include "oatpp/web/protocol/http/incoming/SimpleBodyDecoder.hpp"
#include "oatpp/data/stream/BufferStream.hpp"

namespace oatpp { namespace web { namespace server {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Components

HttpProcessor::Components::Components(const std::shared_ptr<HttpRouter>& pRouter,
                                      const std::shared_ptr<protocol::http::encoding::ProviderCollection>& pContentEncodingProviders,
                                      const std::shared_ptr<const oatpp::web::protocol::http::incoming::BodyDecoder>& pBodyDecoder,
                                      const std::shared_ptr<handler::ErrorHandler>& pErrorHandler,
                                      const RequestInterceptors& pRequestInterceptors,
                                      const ResponseInterceptors& pResponseInterceptors,
                                      const std::shared_ptr<Config>& pConfig)
  : router(pRouter)
  , contentEncodingProviders(pContentEncodingProviders)
  , bodyDecoder(pBodyDecoder)
  , errorHandler(pErrorHandler)
  , requestInterceptors(pRequestInterceptors)
  , responseInterceptors(pResponseInterceptors)
  , config(pConfig)
{}

HttpProcessor::Components::Components(const std::shared_ptr<HttpRouter>& pRouter)
  : Components(pRouter,
               nullptr,
               std::make_shared<oatpp::web::protocol::http::incoming::SimpleBodyDecoder>(),
               std::make_shared<handler::DefaultErrorHandler>(),
               {},
               {},
               std::make_shared<Config>())
{}

HttpProcessor::Components::Components(const std::shared_ptr<HttpRouter>& pRouter, const std::shared_ptr<Config>& pConfig)
  : Components(pRouter,
               nullptr,
               std::make_shared<oatpp::web::protocol::http::incoming::SimpleBodyDecoder>(),
               std::make_shared<handler::DefaultErrorHandler>(),
               {},
               {},
               pConfig)
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other

HttpProcessor::ProcessingResources::ProcessingResources(const std::shared_ptr<Components>& pComponents,
                                                        const provider::ResourceHandle<oatpp::data::stream::IOStream>& pConnection)
  : components(pComponents)
  , connection(pConnection)
  , headersInBuffer(components->config->headersInBufferInitial)
  , headersOutBuffer(components->config->headersOutBufferInitial)
  , headersReader(&headersInBuffer, components->config->headersReaderChunkSize, components->config->headersReaderMaxSize)
  , inStream(data::stream::InputStreamBufferedProxy::createShared(connection.object, std::make_shared<std::string>(data::buffer::IOBuffer::BUFFER_SIZE, 0)))
{}

std::shared_ptr<protocol::http::outgoing::Response>
HttpProcessor::processNextRequest(ProcessingResources& resources,
                                  const std::shared_ptr<protocol::http::incoming::Request>& request,
                                  ConnectionState& connectionState)
{

  std::shared_ptr<protocol::http::outgoing::Response> response;

  try {
    try {

      for (auto &interceptor: resources.components->requestInterceptors) {
        response = interceptor->intercept(request);
        if (response) {
          return response;
        }
      }

      auto route = resources.components->router->getRoute(request->getStartingLine().method,
                                                          request->getStartingLine().path);

      if (!route) {
        data::stream::BufferOutputStream ss;
        ss << "No mapping for HTTP-method: '" << request->getStartingLine().method.toString()
           << "', URL: '" << request->getStartingLine().path.toString() << "'";

        throw oatpp::web::protocol::http::HttpError(protocol::http::Status::CODE_404, ss.toString());
      }

      request->setPathVariables(route.getMatchMap());
      return route.getEndpoint()->handle(request);

    } catch (...) {
      std::throw_with_nested(HttpServerError(request, "Error processing request"));
    }
  } catch (...) {
    response = resources.components->errorHandler->handleError(std::current_exception());
    connectionState = ConnectionState::CLOSING;
  }

  return response;

}

HttpProcessor::ConnectionState HttpProcessor::processNextRequest(ProcessingResources& resources) {

  oatpp::web::protocol::http::HttpError::Info error;
  auto headersReadResult = resources.headersReader.readHeaders(resources.inStream.get(), error);

  if(error.ioStatus <= 0) {
    return ConnectionState::DEAD;
  }

  ConnectionState connectionState = ConnectionState::ALIVE;
  std::shared_ptr<protocol::http::incoming::Request> request;
  std::shared_ptr<protocol::http::outgoing::Response> response;

  if(error.status.code != 0) {
    HttpServerError httpError(nullptr, "Invalid Request Headers");
    auto ePtr = std::make_exception_ptr(httpError);
    response = resources.components->errorHandler->handleError(ePtr);
    connectionState = ConnectionState::CLOSING;
  } else {

    request = protocol::http::incoming::Request::createShared(resources.connection.object,
                                                              headersReadResult.startingLine,
                                                              headersReadResult.headers,
                                                              resources.inStream,
                                                              resources.components->bodyDecoder);

    response = processNextRequest(resources, request, connectionState);

    try {
      try {

        for (auto &interceptor: resources.components->responseInterceptors) {
          response = interceptor->intercept(request, response);
          if (!response) {
            throw protocol::http::HttpError(protocol::http::Status::CODE_500, "Response Interceptor returned an Invalid Response - 'null'");
          }
        }

      } catch (...) {
        std::throw_with_nested(HttpServerError(request, "Error processing request during response interception"));
      }
    } catch (...) {
      response = resources.components->errorHandler->handleError(std::current_exception());
      connectionState = ConnectionState::CLOSING;
    }

    response->putHeaderIfNotExists(protocol::http::Header::SERVER, protocol::http::Header::Value::SERVER);
    protocol::http::utils::CommunicationUtils::considerConnectionState(request, response, connectionState);

    switch(connectionState) {

      case ConnectionState::ALIVE: {
        response->putHeaderIfNotExists(protocol::http::Header::CONNECTION, protocol::http::Header::Value::CONNECTION_KEEP_ALIVE);
        if(!request->hasReadBody()) {
          auto contentLengthStr = headersReadResult.headers.getAsMemoryLabel<data::share::StringKeyLabel>(oatpp::web::protocol::http::Header::CONTENT_LENGTH);
          if(contentLengthStr) {
            bool success;
            auto contentLength = utils::conversion::strToInt64(contentLengthStr.toString(), success);
            if (success && contentLength > 0) {
              resources.inStream->commitReadOffset(contentLength);
            }
          } else {
            resources.inStream->commitReadOffset(resources.inStream->availableToRead());
          }
        }
        break;
      }

      case ConnectionState::CLOSING:
      case ConnectionState::DEAD:
        response->putHeaderIfNotExists(protocol::http::Header::CONNECTION, protocol::http::Header::Value::CONNECTION_CLOSE);
        break;

      case ConnectionState::DELEGATED:
      default:
        break;

    }

  }

  auto contentEncoderProvider =
    protocol::http::utils::CommunicationUtils::selectEncoder(request, resources.components->contentEncodingProviders);

  response->send(resources.connection.object.get(), &resources.headersOutBuffer, contentEncoderProvider.get());

  /* Delegate connection handling to another handler only after the response is sent to the client */
  if(connectionState == ConnectionState::DELEGATED) {
    auto handler = response->getConnectionUpgradeHandler();
    if(handler) {
      handler->handleConnection(resources.connection, response->getConnectionUpgradeParameters());
      connectionState = ConnectionState::DELEGATED;
    } else {
      OATPP_LOGw("[oatpp::web::server::HttpProcessor::processNextRequest()]", "Warning. ConnectionUpgradeHandler not set!")
      connectionState = ConnectionState::CLOSING;
    }
  }

  return connectionState;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Task

HttpProcessor::Task::Task(const std::shared_ptr<Components>& components,
                          const provider::ResourceHandle<oatpp::data::stream::IOStream>& connection,
                          TaskProcessingListener* taskListener)
  : m_components(components)
  , m_connection(connection)
  , m_taskListener(taskListener)
{
  m_taskListener->onTaskStart(m_connection);
}

HttpProcessor::Task::Task(HttpProcessor::Task &&other)
  : m_components(std::move(other.m_components))
  , m_connection(std::move(other.m_connection))
  , m_taskListener(other.m_taskListener)
{
  other.m_taskListener = nullptr;
}

HttpProcessor::Task::~Task() {
  if (m_taskListener != nullptr) {
    m_taskListener->onTaskEnd(m_connection);
  }
}

HttpProcessor::Task &HttpProcessor::Task::operator=(HttpProcessor::Task &&other) {
  m_components = std::move(other.m_components);
  m_connection = std::move(other.m_connection);
  m_taskListener = other.m_taskListener;
  other.m_taskListener = nullptr;
  return *this;
}

void HttpProcessor::Task::run(){

  m_connection.object->initContexts();

  ProcessingResources resources(m_components, m_connection);

  ConnectionState connectionState;

  try {

    do {

      connectionState = HttpProcessor::processNextRequest(resources);

    } while (connectionState == ConnectionState::ALIVE);

  } catch (...) {
    // DO NOTHING
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HttpProcessor::Coroutine

HttpProcessor::Coroutine::Coroutine(const std::shared_ptr<Components>& components,
                                    const provider::ResourceHandle<oatpp::data::stream::IOStream>& connection,
                                    TaskProcessingListener* taskListener)
  : m_components(components)
  , m_connection(connection)
  , m_headersInBuffer(components->config->headersInBufferInitial)
  , m_headersReader(&m_headersInBuffer, components->config->headersReaderChunkSize, components->config->headersReaderMaxSize)
  , m_headersOutBuffer(std::make_shared<oatpp::data::stream::BufferOutputStream>(components->config->headersOutBufferInitial))
  , m_inStream(data::stream::InputStreamBufferedProxy::createShared(m_connection.object, std::make_shared<std::string>(data::buffer::IOBuffer::BUFFER_SIZE, 0)))
  , m_connectionState(ConnectionState::ALIVE)
  , m_taskListener(taskListener)
  , m_shouldInterceptResponse(false)
{
  m_taskListener->onTaskStart(m_connection);
}

HttpProcessor::Coroutine::~Coroutine() {
  m_taskListener->onTaskEnd(m_connection);
}

HttpProcessor::Coroutine::Action HttpProcessor::Coroutine::act() {
  return m_connection.object->initContextsAsync().next(yieldTo(&HttpProcessor::Coroutine::parseHeaders));
}

HttpProcessor::Coroutine::Action HttpProcessor::Coroutine::parseHeaders() {
  m_shouldInterceptResponse = true;
  return m_headersReader.readHeadersAsync(m_inStream).callbackTo(&HttpProcessor::Coroutine::onHeadersParsed);
}

oatpp::async::Action HttpProcessor::Coroutine::onHeadersParsed(const RequestHeadersReader::Result& headersReadResult) {

  m_currentRequest = protocol::http::incoming::Request::createShared(m_connection.object,
                                                                     headersReadResult.startingLine,
                                                                     headersReadResult.headers,
                                                                     m_inStream,
                                                                     m_components->bodyDecoder);

  for(auto& interceptor : m_components->requestInterceptors) {
    m_currentResponse = interceptor->intercept(m_currentRequest);
    if(m_currentResponse) {
      return yieldTo(&HttpProcessor::Coroutine::onResponseFormed);
    }
  }

  m_currentRoute = m_components->router->getRoute(headersReadResult.startingLine.method.toString(), headersReadResult.startingLine.path.toString());

  if(!m_currentRoute) {

    data::stream::BufferOutputStream ss;
    ss << "No mapping for HTTP-method: '" << headersReadResult.startingLine.method.toString()
       << "', URL: '" << headersReadResult.startingLine.path.toString() << "'";
    throw oatpp::web::protocol::http::HttpError(protocol::http::Status::CODE_404, ss.toString());
//    auto eptr = std::make_exception_ptr(error);
//    m_currentResponse = m_components->errorHandler->handleError(eptr);
//    m_connectionState = ConnectionState::CLOSING;
//    return yieldTo(&HttpProcessor::Coroutine::onResponseFormed);
  }

  m_currentRequest->setPathVariables(m_currentRoute.getMatchMap());

  return yieldTo(&HttpProcessor::Coroutine::onRequestFormed);

}

HttpProcessor::Coroutine::Action HttpProcessor::Coroutine::onRequestFormed() {
  return m_currentRoute.getEndpoint()->handleAsync(m_currentRequest).callbackTo(&HttpProcessor::Coroutine::onResponse);
}

HttpProcessor::Coroutine::Action HttpProcessor::Coroutine::onResponse(const std::shared_ptr<protocol::http::outgoing::Response>& response) {
  m_currentResponse = response;
  return yieldTo(&HttpProcessor::Coroutine::onResponseFormed);
}
  
HttpProcessor::Coroutine::Action HttpProcessor::Coroutine::onResponseFormed() {

  if(m_shouldInterceptResponse) {
    m_shouldInterceptResponse = false;
    for (auto &interceptor: m_components->responseInterceptors) {
      m_currentResponse = interceptor->intercept(m_currentRequest, m_currentResponse);
      if (!m_currentResponse) {
        throw oatpp::web::protocol::http::HttpError(protocol::http::Status::CODE_500,
                                                    "Response Interceptor returned an Invalid Response - 'null'");
        //auto eptr = std::make_exception_ptr(error);
        //m_currentResponse = m_components->errorHandler->handleError(eptr);
      }
    }
  }

  m_currentResponse->putHeaderIfNotExists(protocol::http::Header::SERVER, protocol::http::Header::Value::SERVER);
  oatpp::web::protocol::http::utils::CommunicationUtils::considerConnectionState(m_currentRequest, m_currentResponse, m_connectionState);

  switch(m_connectionState) {

    case ConnectionState::ALIVE: {
      m_currentResponse->putHeaderIfNotExists(protocol::http::Header::CONNECTION, protocol::http::Header::Value::CONNECTION_KEEP_ALIVE);
      if(!m_currentRequest->hasReadBody()) {
        auto contentLengthStr = m_currentRequest->getHeaders().getAsMemoryLabel<data::share::StringKeyLabel>(oatpp::web::protocol::http::Header::CONTENT_LENGTH);
        if(contentLengthStr) {
          bool success;
          auto contentLength = utils::conversion::strToInt64(contentLengthStr.toString(), success);
          if (success && contentLength > 0) {
            m_inStream->commitReadOffset(contentLength);
          }
        } else {
          m_inStream->commitReadOffset(m_inStream->availableToRead());
        }
      }
      break;
    }

    case ConnectionState::CLOSING:
    case ConnectionState::DEAD:
      m_currentResponse->putHeaderIfNotExists(protocol::http::Header::CONNECTION, protocol::http::Header::Value::CONNECTION_CLOSE);
      break;

    case ConnectionState::DELEGATED:
    default:
      break;

  }

  auto contentEncoderProvider =
    protocol::http::utils::CommunicationUtils::selectEncoder(m_currentRequest, m_components->contentEncodingProviders);

  return protocol::http::outgoing::Response::sendAsync(m_currentResponse, m_connection.object, m_headersOutBuffer, contentEncoderProvider)
         .next(yieldTo(&HttpProcessor::Coroutine::onRequestDone));

}
  
HttpProcessor::Coroutine::Action HttpProcessor::Coroutine::onRequestDone() {

  switch (m_connectionState) {
    case ConnectionState::ALIVE:
      return yieldTo(&HttpProcessor::Coroutine::parseHeaders);

    /* Delegate connection handling to another handler only after the response is sent to the client */
    case ConnectionState::DELEGATED: {
      auto handler = m_currentResponse->getConnectionUpgradeHandler();
      if(handler) {
        handler->handleConnection(m_connection, m_currentResponse->getConnectionUpgradeParameters());
        m_connectionState = ConnectionState::DELEGATED;
      } else {
        OATPP_LOGw("[oatpp::web::server::HttpProcessor::Coroutine::onResponseFormed()]", "Warning. ConnectionUpgradeHandler not set!")
        m_connectionState = ConnectionState::CLOSING;
      }
      break;
    }

    case ConnectionState::CLOSING:
    case ConnectionState::DEAD:
    default:
      break;

  }
  
  return finish();

}
  
HttpProcessor::Coroutine::Action HttpProcessor::Coroutine::handleError(Error* error) {

  if(error) {

    if(error->is<oatpp::AsyncIOError>()) {
      auto aioe = static_cast<oatpp::AsyncIOError*>(error);
      if(aioe->getCode() == oatpp::IOError::BROKEN_PIPE) {
        return aioe; // do not report BROKEN_PIPE error
      }
    }

//    if(m_currentResponse) {
//      //OATPP_LOGe("[oatpp::web::server::HttpProcessor::Coroutine::handleError()]", "Unhandled error. '{}'. Dropping connection", error->what())
//      return error;
//    }


    std::exception_ptr ePtr = error->getExceptionPtr();
    if(!ePtr) {
      ePtr = std::make_exception_ptr(*error);
    }

    try {
      try {
        std::rethrow_exception(ePtr);
      } catch (...) {
        std::throw_with_nested(HttpServerError(m_currentRequest, "Error processing async request"));
      }
    } catch (...) {
      ePtr = std::current_exception();
      m_currentResponse = m_components->errorHandler->handleError(ePtr);
      if (m_currentResponse != nullptr) {
        return yieldTo(&HttpProcessor::Coroutine::onResponseFormed);
      }
    }

    return new async::Error(ePtr);

//    oatpp::web::protocol::http::HttpError httpError(protocol::http::Status::CODE_500, error->what());
//    auto eptr = std::make_exception_ptr(httpError);
//    m_currentResponse = m_components->errorHandler->handleError(eptr);
//    return yieldTo(&HttpProcessor::Coroutine::onResponseFormed);

  }

  return error;

}
  
}}}
