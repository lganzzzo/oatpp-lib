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

#ifndef oatpp_test_web_app_Controller_hpp
#define oatpp_test_web_app_Controller_hpp

#include "./DTOs.hpp"

#include "oatpp/web/mime/multipart/FileProvider.hpp"
#include "oatpp/web/mime/multipart/InMemoryDataProvider.hpp"
#include "oatpp/web/mime/multipart/Reader.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"

#include "oatpp/web/protocol/http/outgoing/MultipartBody.hpp"
#include "oatpp/web/protocol/http/outgoing/StreamingBody.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/json/ObjectMapper.hpp"

#include "oatpp/data/resource/File.hpp"
#include "oatpp/data/stream/FileStream.hpp"
#include "oatpp/utils/Conversion.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include <sstream>
#include <thread>

namespace oatpp { namespace test { namespace web { namespace app {

namespace multipart = oatpp::web::mime::multipart;

#include OATPP_CODEGEN_BEGIN(ApiController)

class Controller : public oatpp::web::server::api::ApiController {
private:
  static constexpr const char* TAG = "test::web::app::Controller";
public:
  Controller(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
    , available(true)
  {}
public:

  static std::shared_ptr<Controller> createShared(const std::shared_ptr<ObjectMapper>& objectMapper = OATPP_GET_COMPONENT(std::shared_ptr<ObjectMapper>)){
    return std::make_shared<Controller>(objectMapper);
  }

  std::atomic<bool> available;

  ENDPOINT("GET", "/", root) {
    //OATPP_LOGd(TAG, "GET ROOT")
    return createResponse(Status::CODE_200, "Hello World!!!");
  }

  ENDPOINT("GET", "/availability", availability) {
    //OATPP_LOGv(TAG, "GET '/availability'")
    if(available) {
      return createResponse(Status::CODE_200, "Hello World!!!");
    }
    OATPP_LOGi(TAG, "GET '/availability'. Service unavailable.")
    OATPP_ASSERT_HTTP(false, Status::CODE_503, "Service unavailable")
  }

  ADD_CORS(cors)
  ENDPOINT("GET", "/cors", cors) {
    return createResponse(Status::CODE_200, "Ping");
  }

  ADD_CORS(corsOrigin, "127.0.0.1")
  ENDPOINT("GET", "/cors-origin", corsOrigin) {
    return createResponse(Status::CODE_200, "Pong");
  }

  ADD_CORS(corsOriginMethods, "127.0.0.1", "GET, OPTIONS")
  ENDPOINT("GET", "/cors-origin-methods", corsOriginMethods) {
    return createResponse(Status::CODE_200, "Ping");
  }

  ADD_CORS(corsOriginMethodsHeaders, "127.0.0.1", "GET, OPTIONS", "X-PWNT")
  ENDPOINT("GET", "/cors-origin-methods-headers", corsOriginMethodsHeaders) {
    return createResponse(Status::CODE_200, "Pong");
  }

  ENDPOINT("GET", "params/{param}", getWithParams,
           PATH(String, param)) {
    //OATPP_LOGv(TAG, "GET params/{}", param)
    auto dto = TestDto::createShared();
    dto->testValue = param;
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT("GET", "queries", getWithQueries,
           QUERY(String, name), QUERY(Int32, age)) {
    auto dto = TestDto::createShared();
    dto->testValue = "name=" + name + "&age=" + oatpp::utils::Conversion::int32ToStr(*age);
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT("GET", "queries/optional", getWithOptQueries,
           QUERY(String, name, "name", "Default"), QUERY(Int32, age, "age", 101)) {
    auto dto = TestDto::createShared();
    dto->testValue = "name=" + name + "&age=" + oatpp::utils::Conversion::int32ToStr(*age);
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT("GET", "queries/map", getWithQueriesMap,
           QUERIES(QueryParams, queries)) {
    auto dto = TestDto::createShared();
    dto->testMap = dto->testMap.createShared();
    for(auto& it : queries.getAll()) {
      dto->testMap[it.first.toString()] = it.second.toString();
    }
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT("GET", "headers", getWithHeaders,
           HEADER(String, param, "X-TEST-HEADER")) {
    //OATPP_LOGv(TAG, "GET headers {X-TEST-HEADER: {}}", param)
    auto dto = TestDto::createShared();
    dto->testValue = param;
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT("POST", "body", postBody,
           BODY_STRING(String, body)) {
    //OATPP_LOGv(TAG, "POST body {}", body)
    auto dto = TestDto::createShared();
    dto->testValue = body;
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT("POST", "body-dto", postBodyDto,
           BODY_DTO(Object<TestDto>, body)) {
    //OATPP_LOGv(TAG, "POST body {}", body->c_str())
    return createDtoResponse(Status::CODE_200, body);
  }

  ENDPOINT("POST", "echo", echo,
           BODY_STRING(String, body)) {
    //OATPP_LOGv(TAG, "POST body(echo) size={}", body->getSize())
    return createResponse(Status::CODE_200, body);
  }

  ENDPOINT("POST", "testBodyIsNull1", testBodyIsNull1,
           BODY_DTO(String, body)) {
    return createResponse(Status::CODE_200, "OK---" + body);
  }

  ENDPOINT("POST", "testBodyIsNull2", testBodyIsNull2,
           BODY_STRING(String, body)) {
    return createResponse(Status::CODE_200, "OK---" + body);
  }

  ENDPOINT("GET", "header-value-set", headerValueSet,
           HEADER(String, valueSet, "X-VALUE-SET")) {

    oatpp::web::protocol::http::HeaderValueData valueData;
    oatpp::web::protocol::http::Parser::parseHeaderValueData(valueData, valueSet, ',');

    OATPP_ASSERT_HTTP(valueData.tokens.find("VALUE_1") != valueData.tokens.end(), Status::CODE_400, "No header 'VALUE_1' in value set")
    OATPP_ASSERT_HTTP(valueData.tokens.find("VALUE_2") != valueData.tokens.end(), Status::CODE_400, "No header 'VALUE_2' in value set")
    OATPP_ASSERT_HTTP(valueData.tokens.find("VALUE_3") != valueData.tokens.end(), Status::CODE_400, "No header 'VALUE_3' in value set")
    return createResponse(Status::CODE_200, "");
  }

  ENDPOINT("PUT", "/test-with-body", testWithBody,
           BODY_STRING(String, body)) {
    return createResponse(Status::CODE_200, "OK---" + body);
  }

  class ReadCallback : public oatpp::data::stream::ReadCallback {
  private:
    oatpp::String m_text;
    v_int32 m_counter;
    v_int32 m_iterations;
    data::buffer::InlineWriteData m_inlineData;
  public:

    ReadCallback(const oatpp::String& text, v_int32 iterations)
      : m_text(text)
      , m_counter(0)
      , m_iterations(iterations)
      , m_inlineData(text->data(), static_cast<v_buff_size>(text->size()))
    {}

    v_io_size read(void *buffer, v_buff_size count, async::Action& action) override {

      if(m_counter < m_iterations) {

        v_buff_size desiredToRead = m_inlineData.bytesLeft;

        if (desiredToRead > 0) {

          if (desiredToRead > count) {
            desiredToRead = count;
          }

          std::memcpy(buffer, m_inlineData.currBufferPtr, static_cast<size_t>(desiredToRead));
          m_inlineData.inc(desiredToRead);

          if (m_inlineData.bytesLeft == 0) {
            m_inlineData.set(m_text->data(), static_cast<v_buff_size>(m_text->size()));
            m_counter++;
          }

          return desiredToRead;

        }

      }

      return 0;

    }

  };

  ENDPOINT("GET", "chunked/{text-value}/{num-iterations}", chunked,
           PATH(String, text, "text-value"),
           PATH(Int32, numIterations, "num-iterations"),
           REQUEST(std::shared_ptr<IncomingRequest>, request))
  {
    auto body = std::make_shared<oatpp::web::protocol::http::outgoing::StreamingBody>
      (std::make_shared<ReadCallback>(text, *numIterations));
    return OutgoingResponse::createShared(Status::CODE_200, body);
  }

  ENDPOINT("POST", "test/multipart/{chunk-size}", multipartTest,
           PATH(Int32, chunkSize, "chunk-size"),
           REQUEST(std::shared_ptr<IncomingRequest>, request))
  {

    auto multipart = std::make_shared<oatpp::web::mime::multipart::PartList>(request->getHeaders());

    oatpp::web::mime::multipart::Reader multipartReader(multipart.get());
    multipartReader.setDefaultPartReader(oatpp::web::mime::multipart::createInMemoryPartReader(10));

    request->transferBody(&multipartReader);

    auto responseBody = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);

    return OutgoingResponse::createShared(Status::CODE_200, responseBody);

  }

  ENDPOINT("POST", "test/multipart-all", multipartFileTest,
           REQUEST(std::shared_ptr<IncomingRequest>, request))
  {

    /* Prepare multipart container. */
    auto multipart = std::make_shared<multipart::PartList>(request->getHeaders());

    /* Create multipart reader. */
    multipart::Reader multipartReader(multipart.get());

    /* Configure to read part with name "part1" into memory */
    multipartReader.setPartReader("part1", multipart::createInMemoryPartReader(256 /* max-data-size */));

    /* Configure to stream part with name "part2" to file */
    multipartReader.setPartReader("part2", multipart::createFilePartReader("/Users/leonid/Documents/test/my-text-file.tf"));

    /* Configure to read all other parts into memory */
    multipartReader.setDefaultPartReader(multipart::createInMemoryPartReader(16 * 1024 /* max-data-size */));

    /* Read multipart body */
    request->transferBody(&multipartReader);

    /* Print number of uploaded parts */
    OATPP_LOGd("Multipart", "parts_count={}", multipart->count())

    /* Print value of "part1" */
    auto part1 = multipart->getNamedPart("part1");

    OATPP_ASSERT_HTTP(part1, Status::CODE_400, "part1 is empty")

    OATPP_LOGd("Multipart", "part1='{}'", part1->getPayload()->getInMemoryData())

    /* Get part by name "part2"*/
    auto filePart = multipart->getNamedPart("part2");

    OATPP_ASSERT_HTTP(filePart, Status::CODE_400, "part2 is empty")

    auto inputStream = filePart->getPayload()->openInputStream();

    // TODO - process file stream.

    return createResponse(Status::CODE_200, "OK");

  }

  class MPStream : public oatpp::web::mime::multipart::Multipart {
  public:
    typedef oatpp::web::mime::multipart::Part Part;
  private:
    v_uint32 counter = 0;
  public:

    MPStream()
      : oatpp::web::mime::multipart::Multipart(generateRandomBoundary())
    {}

    std::shared_ptr<Part> readNextPart(async::Action& action) override {

      if(counter == 10) {
        return nullptr;
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));

      auto part = std::make_shared<Part>();
      part->putHeader(Header::CONTENT_TYPE, "text/html");

      oatpp::String frameData;

//      if(counter % 2 == 0) {
//        frameData = "<html><body>0</body></html>";
//      } else {
//        frameData = "<html><body>1</body></html>";
//      }
//      part->setDataInfo(std::make_shared<oatpp::data::stream::BufferInputStream>(frameData));

      if(counter % 2 == 0) {
        part->setPayload(std::make_shared<data::resource::File>("/Users/leonid/Documents/test/frame1.jpg"));
      } else {
        part->setPayload(std::make_shared<data::resource::File>("/Users/leonid/Documents/test/frame2.jpg"));
      }

      ++ counter;

      OATPP_LOGd("Multipart", "Frame sent!")

      return part;

    }

    void writeNextPart(const std::shared_ptr<Part>& part, async::Action& action) override {
      throw std::runtime_error("No writes here!!!");
    }

  };

  ENDPOINT("GET", "multipart-stream", multipartStream) {
    auto multipart = std::make_shared<MPStream>();
    auto body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(
      multipart,
      "multipart/x-mixed-replace",
      true /* flush parts */
    );
    return OutgoingResponse::createShared(Status::CODE_200, body);
  }

  ENDPOINT("GET", "enum/as-string", testEnumString,
           HEADER(Enum<AllowedPathParams>::AsString, enumValue, "enum"))
  {
    return createResponse(Status::CODE_200, "");
  }

  ENDPOINT("GET", "enum/as-number", testEnumNumber,
           HEADER(Enum<AllowedPathParams>::AsNumber, enumValue, "enum"))
  {
    return createResponse(Status::CODE_200, "");
  }

  ENDPOINT("GET", "default_headers", getDefaultHeaders1,
           HEADER(String, header, "X-DEFAULT"))
  {
    if(header == "hello_1") {
      return createResponse(Status::CODE_200, "");
    }
    return createResponse(Status::CODE_400, "");
  }

  ENDPOINT("GET", "default_headers/{param}", getDefaultHeaders2,
           HEADER(String, header, "X-DEFAULT"),
           PATH(String, param))
  {
    if(header == "hello_2") {
      return createResponse(Status::CODE_200, "");
    }
    return createResponse(Status::CODE_400, "");
  }

  ENDPOINT_INTERCEPTOR(getBundle, middleware) {
    request->putBundleData("str_param", oatpp::String("str-param"));
    request->putBundleData("int_param", oatpp::Int32(32000));
    return (this->*intercepted)(request);
  }
  ENDPOINT("GET", "bundle", getBundle,
           BUNDLE(String, str_param),
           BUNDLE(Int32, a, "int_param"))
  {
    auto dto = TestDto::createShared();
    dto->testValue = str_param;
    dto->testValueInt = a;
    return createDtoResponse(Status::CODE_200, dto);
  }

  ENDPOINT("GET", "host_header", getHostHeader,
           REQUEST(std::shared_ptr<IncomingRequest>, request)) {
    auto hostHeader = request->getHeader("Host");
    if(hostHeader) {
      return createResponse(Status::CODE_200, hostHeader);
    }
    return createResponse(Status::CODE_400, "");
  }

};

#include OATPP_CODEGEN_END(ApiController)

}}}}

#endif /* oatpp_test_web_app_Controller_hpp */
