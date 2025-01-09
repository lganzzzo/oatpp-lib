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

#ifndef oatpp_test_web_app_ControllerAsync_hpp
#define oatpp_test_web_app_ControllerAsync_hpp

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
#include "oatpp/data/stream/Stream.hpp"
#include "oatpp/utils/Conversion.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

namespace oatpp { namespace test { namespace web { namespace app {

namespace multipart = oatpp::web::mime::multipart;

class ControllerAsync : public oatpp::web::server::api::ApiController {
private:
  static constexpr const char* TAG = "test::web::app::ControllerAsync";
public:
  ControllerAsync(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:
  
  static std::shared_ptr<ControllerAsync> createShared(const std::shared_ptr<ObjectMapper>& objectMapper = OATPP_GET_COMPONENT(std::shared_ptr<ObjectMapper>)){
    return std::make_shared<ControllerAsync>(objectMapper);
  }
  
#include OATPP_CODEGEN_BEGIN(ApiController)

  ENDPOINT_ASYNC("GET", "/", Root) {
    
    ENDPOINT_ASYNC_INIT(Root)
    
    Action act() override {
      //OATPP_LOGv(TAG, "GET '/'")
      return _return(controller->createResponse(Status::CODE_200, "Hello World Async!!!"));
    }

  };

  ENDPOINT_ASYNC("GET", "params/{param}", GetWithParams) {

    ENDPOINT_ASYNC_INIT(GetWithParams)

    Action act() override {
      auto param = request->getPathVariable("param");
      //OATPP_LOGv(TAG, "GET params/{}", param)
      auto dto = TestDto::createShared();
      dto->testValue = param;
      return _return(controller->createDtoResponse(Status::CODE_200, dto));
    }

  };

  ENDPOINT_ASYNC("GET", "headers", GetWithHeaders) {

    ENDPOINT_ASYNC_INIT(GetWithHeaders)

    Action act() override {
      auto param = request->getHeader("X-TEST-HEADER");
      OATPP_ASSERT_HTTP(param, Status::CODE_400, "X-TEST-HEADER missing")
      //OATPP_LOGv(TAG, "GET headers {X-TEST-HEADER: {}}", param)
      auto dto = TestDto::createShared();
      dto->testValue = param;
      return _return(controller->createDtoResponse(Status::CODE_200, dto));
    }

  };

  ENDPOINT_ASYNC("POST", "body", PostBody) {

    ENDPOINT_ASYNC_INIT(PostBody)

    Action act() override {
      //OATPP_LOGv(TAG, "POST body. Reading body...")
      return request->readBodyToStringAsync().callbackTo(&PostBody::onBodyRead);
    }

    Action onBodyRead(const String& body) {
      //OATPP_LOGv(TAG, "POST body {}", body)
      auto dto = TestDto::createShared();
      dto->testValue = body;
      return _return(controller->createDtoResponse(Status::CODE_200, dto));
    }

  };

  ENDPOINT_ASYNC("POST", "echo", Echo) {

    ENDPOINT_ASYNC_INIT(Echo)

    Action act() override {
      //OATPP_LOGv(TAG, "POST body(echo). Reading body...")
      return request->readBodyToStringAsync().callbackTo(&Echo::onBodyRead);
    }

    Action onBodyRead(const String& body) {
      //OATPP_LOGv(TAG, "POST echo size={}", body->getSize())
      return _return(controller->createResponse(Status::CODE_200, body));
    }

  };

  ENDPOINT_ASYNC("PUT", "/test-with-body", TestWithBody) {

    ENDPOINT_ASYNC_INIT(TestWithBody)

    Action act() override {
        return request->readBodyToStringAsync().callbackTo(&TestWithBody::onBodyRead);
    }

    Action onBodyRead(const String& body) {
      return _return(controller->createResponse(Status::CODE_200, "OK---" + body));
    }
  };

  ENDPOINT_ASYNC("GET", "chunked/{text-value}/{num-iterations}", Chunked) {

    ENDPOINT_ASYNC_INIT(Chunked)

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

    Action act() override {
      oatpp::String text = request->getPathVariable("text-value");
      auto numIterations = oatpp::utils::Conversion::strToInt32(request->getPathVariable("num-iterations")->c_str());

      auto body = std::make_shared<oatpp::web::protocol::http::outgoing::StreamingBody>
        (std::make_shared<ReadCallback>(text, numIterations));

      return _return(OutgoingResponse::createShared(Status::CODE_200, body));
    }

  };

  ENDPOINT_ASYNC("POST", "test/multipart/{chunk-size}", MultipartTest) {

    ENDPOINT_ASYNC_INIT(MultipartTest)

    v_int32 m_chunkSize;
    std::shared_ptr<oatpp::web::mime::multipart::PartList> m_multipart;

    Action act() override {

      m_chunkSize = oatpp::utils::Conversion::strToInt32(request->getPathVariable("chunk-size")->c_str());

      m_multipart = std::make_shared<oatpp::web::mime::multipart::PartList>(request->getHeaders());
      auto multipartReader = std::make_shared<oatpp::web::mime::multipart::AsyncReader>(m_multipart);

      multipartReader->setDefaultPartReader(oatpp::web::mime::multipart::createAsyncInMemoryPartReader(10));

      return request->transferBodyAsync(multipartReader).next(yieldTo(&MultipartTest::respond));

    }

    Action respond() {

      auto responseBody = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(m_multipart);
      return _return(OutgoingResponse::createShared(Status::CODE_200, responseBody));

    }

  };


  ENDPOINT_ASYNC("POST", "test/multipart-all", MultipartUpload) {

    ENDPOINT_ASYNC_INIT(MultipartUpload)

    /* Coroutine State */
    std::shared_ptr<multipart::PartList> m_multipart;

    Action act() override {

      m_multipart = std::make_shared<multipart::PartList>(request->getHeaders());
      auto multipartReader = std::make_shared<multipart::AsyncReader>(m_multipart);

      /* Configure to read part with name "part1" into memory */
      multipartReader->setPartReader("part1", multipart::createAsyncInMemoryPartReader(256 /* max-data-size */));

      /* Configure to stream part with name "part2" to file */
      multipartReader->setPartReader("part2", multipart::createAsyncFilePartReader("/Users/leonid/Documents/test/my-text-file.tf"));

      /* Configure to read all other parts into memory */
      multipartReader->setDefaultPartReader(multipart::createAsyncInMemoryPartReader(16 * 1024 /* max-data-size */));

      /* Read multipart body */
      return request->transferBodyAsync(multipartReader).next(yieldTo(&MultipartUpload::onUploaded));

    }

    Action onUploaded() {

      /* Print number of uploaded parts */
      OATPP_LOGd("Multipart", "parts_count={}", m_multipart->count())

      /* Get multipart by name */
      auto part1 = m_multipart->getNamedPart("part1");

      /* Asser part not-null */
      OATPP_ASSERT_HTTP(part1, Status::CODE_400, "part1 is empty")

      /* Print value of "part1" */
      OATPP_LOGd("Multipart", "part1='{}'", part1->getPayload()->getInMemoryData())

      /* Get multipart by name */
      auto filePart = m_multipart->getNamedPart("part2");

      /* Asser part not-null */
      OATPP_ASSERT_HTTP(filePart, Status::CODE_400, "part2 is empty")

      auto inputStream = filePart->getPayload()->openInputStream();

      // TODO - process file stream.

      return _return(controller->createResponse(Status::CODE_200, "OK"));

    }

  };


  class MPStream : public oatpp::web::mime::multipart::Multipart {
  public:
    typedef oatpp::web::mime::multipart::Part Part;
  private:
    v_uint32 counter = 0;
    bool m_wait = false;
  public:

    MPStream()
      : oatpp::web::mime::multipart::Multipart(generateRandomBoundary())
    {}

    std::shared_ptr<Part> readNextPart(async::Action& action) override {

      if(counter == 10) {
        return nullptr;
      }

      if(m_wait) {
        m_wait = false;
        action = async::Action::createWaitRepeatAction(1000 * 1000 + oatpp::Environment::getMicroTickCount());
        return nullptr;
      }

      m_wait = true;

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

  ENDPOINT_ASYNC("GET", "multipart-stream", MultipartStream) {

    ENDPOINT_ASYNC_INIT(MultipartStream)

    Action act() override {
      auto multipart = std::make_shared<MPStream>();
      auto body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(
        multipart,
        "multipart/x-mixed-replace",
        true /* flush parts */
      );
      return _return(OutgoingResponse::createShared(Status::CODE_200, body));
    }

  };

  ENDPOINT_ASYNC("GET", "host_header", HostHeader) {

    ENDPOINT_ASYNC_INIT(HostHeader)

    Action act() override {
      auto hostHeader = request->getHeader("Host");
      if(hostHeader) {
        return _return(controller->createResponse(Status::CODE_200, hostHeader));
      }
      return _return(controller->createResponse(Status::CODE_400, ""));
    }

  };

#include OATPP_CODEGEN_END(ApiController)
  
};
  
}}}}

#endif /* oatpp_test_web_app_ControllerAsync_hpp */
