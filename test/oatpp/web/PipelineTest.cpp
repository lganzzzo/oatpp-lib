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

#include "PipelineTest.hpp"

#include "oatpp/web/app/Controller.hpp"

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"

#include "oatpp/json/ObjectMapper.hpp"

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

#include "oatpp/network/virtual_/client/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/Interface.hpp"

#include "oatpp/data/stream/BufferStream.hpp"
#include "oatpp/macro/component.hpp"

#include "oatpp-test/web/ClientServerTestRunner.hpp"

namespace oatpp { namespace test { namespace web {

namespace {

class TestComponent {
private:
  v_uint16 m_port;
public:

  TestComponent(v_uint16 port)
    : m_port(port)
  {}

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, virtualInterface)([] {
    return oatpp::network::virtual_::Interface::obtainShared("virtualhost");
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([this] {

    if(m_port == 0) { // Use oatpp virtual interface
      OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, _interface);
      return std::static_pointer_cast<oatpp::network::ServerConnectionProvider>(
        oatpp::network::virtual_::server::ConnectionProvider::createShared(_interface)
      );
    }

    return std::static_pointer_cast<oatpp::network::ServerConnectionProvider>(
      oatpp::network::tcp::server::ConnectionProvider::createShared({"localhost", m_port})
    );

  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
    return oatpp::web::server::HttpConnectionHandler::createShared(router);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper)([] {
    return std::make_shared<oatpp::json::ObjectMapper>();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider)([this] {

    if(m_port == 0) {
      OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, _interface);
      return std::static_pointer_cast<oatpp::network::ClientConnectionProvider>(
        oatpp::network::virtual_::client::ConnectionProvider::createShared(_interface)
      );
    }

    return std::static_pointer_cast<oatpp::network::ClientConnectionProvider>(
      oatpp::network::tcp::client::ConnectionProvider::createShared({"localhost", m_port})
    );

  }());

};

const char* const SAMPLE_IN_1 =
  "GET / HTTP/1.1\r\n"
  "Connection: keep-alive\r\n"
  "Content-Length: 0\r\n"
  "\r\n";

const char* const SAMPLE_OUT_1 =
  "HTTP/1.1 200 OK\r\n"
  "Content-Length: 14\r\n"
  "Connection: keep-alive\r\n"
  "Server: oatpp/" OATPP_VERSION "\r\n"
  "\r\n"
  "Hello World!!!";

const char *const SAMPLE_IN_2 =
  "PUT /test-with-body HTTP/1.1\r\n"
  "Connection: keep-alive\r\n"
  "Content-Length: 14\r\n"
  "\r\n"
  "Hello World!!!";

const char* const SAMPLE_OUT_2 =
  "HTTP/1.1 200 OK\r\n"
  "Content-Length: 19\r\n"
  "Connection: keep-alive\r\n"
  "Server: oatpp/" OATPP_VERSION "\r\n"
  "\r\n"
  "OK---Hello World!!!";

}

void PipelineTest::onRun() {

  TestComponent component(m_port);

  oatpp::test::web::ClientServerTestRunner runner;

  runner.addController(app::Controller::createShared());

  runner.run([this] {

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider);

    auto connection = clientConnectionProvider->get();
    connection.object->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);

    std::thread pipeInThread1([this, connection] {

      oatpp::data::stream::BufferOutputStream pipelineStream;

      for (v_int32 i = 0; i < m_pipelineSize; i++) {
        pipelineStream << SAMPLE_IN_1;
      }

      auto dataToSend = pipelineStream.toString();
      OATPP_LOGd(TAG, "Sending {} bytes", dataToSend->size())

      oatpp::data::stream::BufferInputStream inputStream(dataToSend);

      oatpp::data::buffer::IOBuffer ioBuffer;

      oatpp::data::stream::transfer(&inputStream, connection.object.get(), 0, ioBuffer.getData(), ioBuffer.getSize());

    });

    std::thread pipeOutThread1([this, connection] {

      oatpp::String sample = SAMPLE_OUT_1;
      oatpp::data::stream::BufferOutputStream receiveStream;
      oatpp::data::buffer::IOBuffer ioBuffer;

      auto transferSize = static_cast<v_io_size>(sample->size() * static_cast<size_t>(m_pipelineSize));

      OATPP_LOGd(TAG, "want to Receive {} bytes", transferSize)
      oatpp::data::stream::transfer(connection.object.get(), &receiveStream, transferSize, ioBuffer.getData(), ioBuffer.getSize());

      auto result = receiveStream.toString();

      OATPP_ASSERT(result->size() == sample->size() * static_cast<size_t>(m_pipelineSize))
      //OATPP_ASSERT(result == wantedResult) // headers may come in different order on different OSs

    });

    pipeInThread1.join();
    pipeOutThread1.join();

    connection = clientConnectionProvider->get();
    connection.object->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);

    std::thread pipeInThread2([this, connection] {

      oatpp::data::stream::BufferOutputStream pipelineStream;

      for (v_int32 i = 0; i < m_pipelineSize; i++) {
        pipelineStream << SAMPLE_IN_2;
      }

      auto dataToSend = pipelineStream.toString();
      OATPP_LOGD(TAG, "Sending %lu bytes", dataToSend->size())

      oatpp::data::stream::BufferInputStream inputStream(dataToSend);

      oatpp::data::buffer::IOBuffer ioBuffer;

      oatpp::data::stream::transfer(&inputStream, connection.object.get(), 0, ioBuffer.getData(), ioBuffer.getSize());

    });

    std::thread pipeOutThread2([this, connection] {

      oatpp::String sample = SAMPLE_OUT_2;
      oatpp::data::stream::BufferOutputStream receiveStream;
      oatpp::data::buffer::IOBuffer ioBuffer;

      auto transferSize = static_cast<v_io_size>(sample->size() * static_cast<size_t>(m_pipelineSize));

      OATPP_LOGD(TAG, "want to Receive %ld bytes", transferSize)
      oatpp::data::stream::transfer(connection.object.get(), &receiveStream, transferSize, ioBuffer.getData(), ioBuffer.getSize());

      auto result = receiveStream.toString();

      OATPP_ASSERT(result->size() == sample->size() * static_cast<size_t>(m_pipelineSize))
      //OATPP_ASSERT(result == wantedResult) // headers may come in different order on different OSs

    });

    pipeInThread2.join();
    pipeOutThread2.join();

  }, std::chrono::minutes(10));

  std::this_thread::sleep_for(std::chrono::seconds(1));

}

}}}
