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

#include "FullTest.hpp"

#include "oatpp/web/app/Client.hpp"

#include "oatpp/web/app/Controller.hpp"

#include "oatpp/web/client/HttpRequestExecutor.hpp"

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/network/server/SimpleTCPConnectionProvider.hpp"
#include "oatpp/network/client/SimpleTCPConnectionProvider.hpp"

#include "oatpp/network/virtual_/client/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/Interface.hpp"

#include "oatpp/core/macro/component.hpp"

#include "oatpp-test/web/ClientServerTestRunner.hpp"

namespace oatpp { namespace test { namespace web {

namespace {

//#define OATPP_TEST_USE_PORT 8123

class TestComponent {
public:

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, virtualInterface)([] {
    return oatpp::network::virtual_::Interface::createShared("virtualhost");
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([this] {
#ifdef OATPP_TEST_USE_PORT
      return oatpp::network::server::SimpleTCPConnectionProvider::createShared(OATPP_TEST_USE_PORT);
#else
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, interface);
    return oatpp::network::virtual_::server::ConnectionProvider::createShared(interface);
#endif
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::server::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
    return oatpp::web::server::HttpConnectionHandler::createShared(router);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper)([] {
    return oatpp::parser::json::mapping::ObjectMapper::createShared();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider)([this] {
#ifdef OATPP_TEST_USE_PORT
      return oatpp::network::client::SimpleTCPConnectionProvider::createShared("127.0.0.1", OATPP_TEST_USE_PORT);
#else
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, interface);
    return oatpp::network::virtual_::client::ConnectionProvider::createShared(interface);
#endif
  }());

};

}
  
void FullTest::onRun() {

  TestComponent component;

  oatpp::test::web::ClientServerTestRunner runner;

  runner.addController(app::Controller::createShared());

  runner.run([] {

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, clientConnectionProvider);
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);

    auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(clientConnectionProvider);
    auto client = app::Client::createShared(requestExecutor, objectMapper);

    auto connection = client->getConnection();

    v_int32 iterationsStep = 1000;

    for(v_int32 i = 0; i < iterationsStep * 10; i ++) {

      { // test simple GET
        auto response = client->getRoot(connection);
        auto value = response->readBodyToString();
        OATPP_ASSERT(value == "Hello World!!!");
      }

      { // test GET with path parameter
        auto response = client->getWithParams("my_test_param", connection);
        auto dto = response->readBodyToDto<app::TestDto>(objectMapper);
        OATPP_ASSERT(dto);
        OATPP_ASSERT(dto->testValue == "my_test_param");
      }
      
      { // test GET with query parameters
        auto response = client->getWithQueries("oatpp", 1);
        auto dto = response->readBodyToDto<app::TestDto>(objectMapper);
        OATPP_ASSERT(dto);
        OATPP_ASSERT(dto->testValue == "name=oatpp&age=1");
      }

      { // test GET with header parameter
        auto response = client->getWithHeaders("my_test_header", connection);
        auto dto = response->readBodyToDto<app::TestDto>(objectMapper);
        OATPP_ASSERT(dto);
        OATPP_ASSERT(dto->testValue == "my_test_header");
      }

      { // test POST with body
        auto response = client->postBody("my_test_body", connection);
        auto dto = response->readBodyToDto<app::TestDto>(objectMapper);
        OATPP_ASSERT(dto);
        OATPP_ASSERT(dto->testValue == "my_test_body");
      }

      { // test Big Echo with body
        oatpp::data::stream::ChunkedBuffer stream;
        for(v_int32 i = 0; i < oatpp::data::buffer::IOBuffer::BUFFER_SIZE; i++) {
          stream.write("0123456789", 10);
        }
        auto data = stream.toString();
        auto response = client->echoBody(data, connection);
        auto returnedData = response->readBodyToString();
        OATPP_ASSERT(returnedData);
        OATPP_ASSERT(returnedData == data);
      }

      if((i + 1) % iterationsStep == 0) {
        OATPP_LOGD("i", "%d", i + 1);
      }

    }

  }, std::chrono::minutes(10));

}
  
}}}
