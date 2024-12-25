
#######################################################################################################
## oatpp

add_library(oatpp
		oatpp/async/ConditionVariable.cpp
		oatpp/async/ConditionVariable.hpp
		oatpp/async/Coroutine.cpp
		oatpp/async/Coroutine.hpp
		oatpp/async/CoroutineWaitList.cpp
		oatpp/async/CoroutineWaitList.hpp
		oatpp/async/Error.cpp
		oatpp/async/Error.hpp
		oatpp/async/Executor.cpp
		oatpp/async/Executor.hpp
		oatpp/async/Lock.cpp
		oatpp/async/Lock.hpp
		oatpp/async/Processor.cpp
		oatpp/async/Processor.hpp
		oatpp/async/utils/FastQueue.hpp
		oatpp/async/worker/IOEventWorker_common.cpp
		oatpp/async/worker/IOEventWorker_epoll.cpp
		oatpp/async/worker/IOEventWorker_kqueue.cpp
		oatpp/async/worker/IOEventWorker_stub.cpp
		oatpp/async/worker/IOEventWorker.hpp
		oatpp/async/worker/IOWorker.cpp
		oatpp/async/worker/IOWorker.hpp
		oatpp/async/worker/TimerWorker.cpp
		oatpp/async/worker/TimerWorker.hpp
		oatpp/async/worker/Worker.cpp
		oatpp/async/worker/Worker.hpp
		oatpp/base/CommandLineArguments.cpp
		oatpp/base/CommandLineArguments.hpp
		oatpp/base/Compiler.hpp
		oatpp/base/Config.hpp
		oatpp/base/Countable.cpp
		oatpp/base/Countable.hpp
		oatpp/base/Log.cpp
		oatpp/base/Log.hpp
		oatpp/base/ObjectHandle.hpp
        oatpp/codegen/api_controller/auth_define.hpp
        oatpp/codegen/api_controller/auth_undef.hpp
        oatpp/codegen/api_controller/base_define.hpp
        oatpp/codegen/api_controller/base_undef.hpp
        oatpp/codegen/api_controller/bundle_define.hpp
        oatpp/codegen/api_controller/bundle_undef.hpp
        oatpp/codegen/api_controller/cors_define.hpp
        oatpp/codegen/api_controller/cors_undef.hpp
        oatpp/codegen/dto/base_define.hpp
        oatpp/codegen/dto/base_undef.hpp
        oatpp/codegen/dto/enum_define.hpp
        oatpp/codegen/dto/enum_undef.hpp
		oatpp/codegen/ApiClient_define.hpp
		oatpp/codegen/ApiClient_undef.hpp
		oatpp/codegen/ApiController_define.hpp
		oatpp/codegen/ApiController_undef.hpp
		oatpp/codegen/DbClient_define.hpp
		oatpp/codegen/DbClient_undef.hpp
		oatpp/codegen/DTO_define.hpp
		oatpp/codegen/DTO_undef.hpp
		oatpp/concurrency/SpinLock.cpp
		oatpp/concurrency/SpinLock.hpp
		oatpp/concurrency/Utils.cpp
		oatpp/concurrency/Utils.hpp
		oatpp/data/Bundle.cpp
		oatpp/data/Bundle.hpp
		oatpp/data/buffer/FIFOBuffer.cpp
		oatpp/data/buffer/FIFOBuffer.hpp
		oatpp/data/buffer/IOBuffer.cpp
		oatpp/data/buffer/IOBuffer.hpp
		oatpp/data/buffer/Processor.cpp
		oatpp/data/buffer/Processor.hpp
		oatpp/data/mapping/ObjectRemapper.cpp
		oatpp/data/mapping/ObjectRemapper.hpp
		oatpp/data/mapping/ObjectMapper.cpp
		oatpp/data/mapping/ObjectMapper.hpp
		oatpp/data/mapping/ObjectToTreeMapper.cpp
		oatpp/data/mapping/ObjectToTreeMapper.hpp
		oatpp/data/mapping/Tree.cpp
		oatpp/data/mapping/Tree.hpp
		oatpp/data/mapping/TreeToObjectMapper.cpp
		oatpp/data/mapping/TreeToObjectMapper.hpp
		oatpp/data/mapping/TypeResolver.cpp
		oatpp/data/mapping/TypeResolver.hpp
		oatpp/data/resource/File.cpp
		oatpp/data/resource/File.hpp
		oatpp/data/resource/InMemoryData.cpp
		oatpp/data/resource/InMemoryData.hpp
		oatpp/data/resource/Resource.hpp
		oatpp/data/resource/TemporaryFile.cpp
		oatpp/data/resource/TemporaryFile.hpp
		oatpp/data/share/LazyStringMap.hpp
		oatpp/data/share/MemoryLabel.cpp
		oatpp/data/share/MemoryLabel.hpp
		oatpp/data/share/StringTemplate.cpp
		oatpp/data/share/StringTemplate.hpp
		oatpp/data/stream/BufferStream.cpp
		oatpp/data/stream/BufferStream.hpp
		oatpp/data/stream/FIFOStream.cpp
		oatpp/data/stream/FIFOStream.hpp
		oatpp/data/stream/FileStream.cpp
		oatpp/data/stream/FileStream.hpp
		oatpp/data/stream/Stream.cpp
		oatpp/data/stream/Stream.hpp
		oatpp/data/stream/StreamBufferedProxy.cpp
		oatpp/data/stream/StreamBufferedProxy.hpp
		oatpp/data/type/Any.cpp
		oatpp/data/type/Any.hpp
		oatpp/data/type/Collection.hpp
		oatpp/data/type/Enum.cpp
		oatpp/data/type/Enum.hpp
		oatpp/data/type/List.cpp
		oatpp/data/type/List.hpp
		oatpp/data/type/Map.hpp
		oatpp/data/type/Object.cpp
		oatpp/data/type/Object.hpp
		oatpp/data/type/PairList.cpp
		oatpp/data/type/PairList.hpp
		oatpp/data/type/Primitive.cpp
		oatpp/data/type/Primitive.hpp
		oatpp/data/type/Tree.cpp
		oatpp/data/type/Tree.hpp
		oatpp/data/type/Type.cpp
		oatpp/data/type/Type.hpp
		oatpp/data/type/UnorderedMap.cpp
		oatpp/data/type/UnorderedMap.hpp
		oatpp/data/type/UnorderedSet.cpp
		oatpp/data/type/UnorderedSet.hpp
		oatpp/data/type/Vector.cpp
		oatpp/data/type/Vector.hpp
        oatpp/encoding/Base64.cpp
        oatpp/encoding/Base64.hpp
        oatpp/encoding/Hex.cpp
        oatpp/encoding/Hex.hpp
        oatpp/encoding/Unicode.cpp
        oatpp/encoding/Unicode.hpp
        oatpp/encoding/Url.cpp
        oatpp/encoding/Url.hpp
		oatpp/json/Beautifier.cpp
		oatpp/json/Beautifier.hpp
		oatpp/json/Deserializer.cpp
		oatpp/json/Deserializer.hpp
		oatpp/json/ObjectMapper.cpp
		oatpp/json/ObjectMapper.hpp
		oatpp/json/Serializer.cpp
		oatpp/json/Serializer.hpp
		oatpp/json/Utils.cpp
		oatpp/json/Utils.hpp
		oatpp/macro/basic.hpp
		oatpp/macro/codegen.hpp
		oatpp/macro/component.hpp
        oatpp/network/Address.cpp
        oatpp/network/Address.hpp
        oatpp/network/ConnectionHandler.hpp
        oatpp/network/ConnectionPool.cpp
        oatpp/network/ConnectionPool.hpp
        oatpp/network/ConnectionProvider.cpp
        oatpp/network/ConnectionProvider.hpp
        oatpp/network/ConnectionProviderSwitch.cpp
        oatpp/network/ConnectionProviderSwitch.hpp
        oatpp/network/Server.cpp
        oatpp/network/Server.hpp
        oatpp/network/Url.cpp
        oatpp/network/Url.hpp
        oatpp/network/monitor/ConnectionInactivityChecker.cpp
        oatpp/network/monitor/ConnectionInactivityChecker.hpp
        oatpp/network/monitor/ConnectionMaxAgeChecker.cpp
        oatpp/network/monitor/ConnectionMaxAgeChecker.hpp
        oatpp/network/monitor/ConnectionMonitor.cpp
        oatpp/network/monitor/ConnectionMonitor.hpp
        oatpp/network/monitor/MetricsChecker.hpp
        oatpp/network/monitor/StatCollector.hpp
        oatpp/network/tcp/Connection.cpp
        oatpp/network/tcp/Connection.hpp
        oatpp/network/tcp/ConnectionConfigurer.hpp
        oatpp/network/tcp/client/ConnectionProvider.cpp
        oatpp/network/tcp/client/ConnectionProvider.hpp
        oatpp/network/tcp/server/ConnectionProvider.cpp
        oatpp/network/tcp/server/ConnectionProvider.hpp
        oatpp/network/virtual_/Interface.cpp
        oatpp/network/virtual_/Interface.hpp
        oatpp/network/virtual_/Pipe.cpp
        oatpp/network/virtual_/Pipe.hpp
        oatpp/network/virtual_/Socket.cpp
        oatpp/network/virtual_/Socket.hpp
        oatpp/network/virtual_/client/ConnectionProvider.cpp
        oatpp/network/virtual_/client/ConnectionProvider.hpp
        oatpp/network/virtual_/server/ConnectionProvider.cpp
        oatpp/network/virtual_/server/ConnectionProvider.hpp
        oatpp/orm/Connection.hpp
        oatpp/orm/DbClient.cpp
        oatpp/orm/DbClient.hpp
        oatpp/orm/Executor.cpp
        oatpp/orm/Executor.hpp
        oatpp/orm/QueryResult.cpp
        oatpp/orm/QueryResult.hpp
        oatpp/orm/SchemaMigration.cpp
        oatpp/orm/SchemaMigration.hpp
        oatpp/orm/Transaction.cpp
        oatpp/orm/Transaction.hpp
		oatpp/provider/Invalidator.hpp
		oatpp/provider/Pool.hpp
		oatpp/provider/Provider.hpp
		oatpp/utils/parser/Caret.cpp
		oatpp/utils/parser/Caret.hpp
		oatpp/utils/parser/ParsingError.cpp
		oatpp/utils/parser/ParsingError.hpp
		oatpp/utils/Binary.cpp
		oatpp/utils/Binary.hpp
		oatpp/utils/Conversion.cpp
		oatpp/utils/Conversion.hpp
		oatpp/utils/CRC32.cpp
		oatpp/utils/CRC32.hpp
		oatpp/utils/Random.cpp
		oatpp/utils/Random.hpp
		oatpp/utils/String.cpp
		oatpp/utils/String.hpp
        oatpp/web/client/ApiClient.cpp
        oatpp/web/client/ApiClient.hpp
        oatpp/web/client/HttpRequestExecutor.cpp
        oatpp/web/client/HttpRequestExecutor.hpp
        oatpp/web/client/RequestExecutor.cpp
        oatpp/web/client/RequestExecutor.hpp
        oatpp/web/client/RetryPolicy.cpp
        oatpp/web/client/RetryPolicy.hpp
        oatpp/web/mime/multipart/FileProvider.cpp
        oatpp/web/mime/multipart/FileProvider.hpp
        oatpp/web/mime/multipart/InMemoryDataProvider.cpp
        oatpp/web/mime/multipart/InMemoryDataProvider.hpp
        oatpp/web/mime/multipart/Multipart.cpp
        oatpp/web/mime/multipart/Multipart.hpp
        oatpp/web/mime/multipart/Part.cpp
        oatpp/web/mime/multipart/Part.hpp
        oatpp/web/mime/multipart/PartList.cpp
        oatpp/web/mime/multipart/PartList.hpp
        oatpp/web/mime/multipart/PartReader.cpp
        oatpp/web/mime/multipart/PartReader.hpp
        oatpp/web/mime/multipart/Reader.cpp
        oatpp/web/mime/multipart/Reader.hpp
        oatpp/web/mime/multipart/StatefulParser.cpp
        oatpp/web/mime/multipart/StatefulParser.hpp
        oatpp/web/mime/multipart/TemporaryFileProvider.cpp
        oatpp/web/mime/multipart/TemporaryFileProvider.hpp
		oatpp/web/mime/ContentMappers.cpp
		oatpp/web/mime/ContentMappers.hpp
        oatpp/web/protocol/CommunicationError.cpp
        oatpp/web/protocol/CommunicationError.hpp
        oatpp/web/protocol/http/Http.cpp
        oatpp/web/protocol/http/Http.hpp
        oatpp/web/protocol/http/encoding/Chunked.cpp
        oatpp/web/protocol/http/encoding/Chunked.hpp
        oatpp/web/protocol/http/encoding/EncoderProvider.hpp
        oatpp/web/protocol/http/encoding/ProviderCollection.cpp
        oatpp/web/protocol/http/encoding/ProviderCollection.hpp
        oatpp/web/protocol/http/incoming/BodyDecoder.cpp
        oatpp/web/protocol/http/incoming/BodyDecoder.hpp
        oatpp/web/protocol/http/incoming/Request.cpp
        oatpp/web/protocol/http/incoming/Request.hpp
        oatpp/web/protocol/http/incoming/RequestHeadersReader.cpp
        oatpp/web/protocol/http/incoming/RequestHeadersReader.hpp
        oatpp/web/protocol/http/incoming/Response.cpp
        oatpp/web/protocol/http/incoming/Response.hpp
        oatpp/web/protocol/http/incoming/ResponseHeadersReader.cpp
        oatpp/web/protocol/http/incoming/ResponseHeadersReader.hpp
        oatpp/web/protocol/http/incoming/SimpleBodyDecoder.cpp
        oatpp/web/protocol/http/incoming/SimpleBodyDecoder.hpp
        oatpp/web/protocol/http/outgoing/Body.cpp
        oatpp/web/protocol/http/outgoing/Body.hpp
        oatpp/web/protocol/http/outgoing/BufferBody.cpp
        oatpp/web/protocol/http/outgoing/BufferBody.hpp
        oatpp/web/protocol/http/outgoing/MultipartBody.cpp
        oatpp/web/protocol/http/outgoing/MultipartBody.hpp
        oatpp/web/protocol/http/outgoing/Request.cpp
        oatpp/web/protocol/http/outgoing/Request.hpp
        oatpp/web/protocol/http/outgoing/Response.cpp
        oatpp/web/protocol/http/outgoing/Response.hpp
        oatpp/web/protocol/http/outgoing/ResponseFactory.cpp
        oatpp/web/protocol/http/outgoing/ResponseFactory.hpp
        oatpp/web/protocol/http/outgoing/StreamingBody.cpp
        oatpp/web/protocol/http/outgoing/StreamingBody.hpp
        oatpp/web/protocol/http/utils/CommunicationUtils.cpp
        oatpp/web/protocol/http/utils/CommunicationUtils.hpp
        oatpp/web/server/AsyncHttpConnectionHandler.cpp
        oatpp/web/server/AsyncHttpConnectionHandler.hpp
        oatpp/web/server/HttpConnectionHandler.cpp
        oatpp/web/server/HttpConnectionHandler.hpp
        oatpp/web/server/HttpProcessor.cpp
        oatpp/web/server/HttpProcessor.hpp
        oatpp/web/server/HttpRequestHandler.hpp
        oatpp/web/server/HttpRouter.cpp
        oatpp/web/server/HttpRouter.hpp
        oatpp/web/server/HttpServerError.cpp
        oatpp/web/server/HttpServerError.hpp
        oatpp/web/server/api/ApiController.cpp
        oatpp/web/server/api/ApiController.hpp
        oatpp/web/server/api/Endpoint.cpp
        oatpp/web/server/api/Endpoint.hpp
        oatpp/web/server/handler/AuthorizationHandler.cpp
        oatpp/web/server/handler/AuthorizationHandler.hpp
        oatpp/web/server/handler/ErrorHandler.cpp
        oatpp/web/server/handler/ErrorHandler.hpp
        oatpp/web/server/interceptor/AllowCorsGlobal.cpp
        oatpp/web/server/interceptor/AllowCorsGlobal.hpp
        oatpp/web/server/interceptor/RequestInterceptor.hpp
        oatpp/web/server/interceptor/ResponseInterceptor.hpp
        oatpp/web/url/mapping/Pattern.cpp
        oatpp/web/url/mapping/Pattern.hpp
        oatpp/web/url/mapping/Router.hpp
		oatpp/Environment.cpp
		oatpp/Environment.hpp
		oatpp/IODefinitions.cpp
		oatpp/IODefinitions.hpp
		oatpp/Types.hpp
)
set_target_source_groups(oatpp STRIP_PREFIX "oatpp")

set_target_properties(oatpp PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        VERSION "${oatpp_VERSION}"
        SOVERSION "${oatpp_VERSION_MAJOR}.${oatpp_VERSION_MINOR}"
)
if (MSVC)
    target_compile_options(oatpp PRIVATE /permissive-)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD ON)

find_package(Threads REQUIRED)

#######################################################################################################
## Link external libraries

SET(OATPP_ADD_LINK_LIBS "")

if(MSVC OR MINGW)
        SET(OATPP_ADD_LINK_LIBS wsock32 ws2_32)
elseif(NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        if(OATPP_LINK_ATOMIC)
                SET(OATPP_ADD_LINK_LIBS atomic)
        endif()
endif()

message("OATPP_ADD_LINK_LIBS=${OATPP_ADD_LINK_LIBS}")

target_link_libraries(oatpp PUBLIC ${CMAKE_THREAD_LIBS_INIT}
        ${OATPP_ADD_LINK_LIBS}
)

target_include_directories(oatpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

#######################################################################################################
## oatpp-test

if(OATPP_LINK_TEST_LIBRARY)

	add_library(oatpp-test
			oatpp-test/Checker.cpp
			oatpp-test/Checker.hpp
			oatpp-test/UnitTest.cpp
			oatpp-test/UnitTest.hpp
			oatpp-test/web/ClientServerTestRunner.hpp
	)
	set_target_source_groups(oatpp-test STRIP_PREFIX oatpp-test)

	set_target_properties(oatpp-test PROPERTIES
			CXX_STANDARD 11
			CXX_EXTENSIONS OFF
			CXX_STANDARD_REQUIRED ON
                        VERSION "${oatpp_VERSION}"
                        SOVERSION "${oatpp_VERSION_MAJOR}.${oatpp_VERSION_MINOR}"
	)
	if (MSVC)
		target_compile_options(oatpp-test PRIVATE /permissive-)
	endif()

	target_link_libraries(oatpp-test PUBLIC oatpp)

	target_include_directories(oatpp-test PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	)

	add_dependencies(oatpp-test oatpp)

endif()

#######################################################################################################
## Install targets

if(OATPP_INSTALL)
    include("../cmake/module-install.cmake")
endif()
