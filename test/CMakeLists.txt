
add_executable(oatppAllTests
        oatpp/async/ConditionVariableTest.cpp
        oatpp/async/ConditionVariableTest.hpp
        oatpp/async/LockTest.cpp
        oatpp/async/LockTest.hpp
        oatpp/base/CommandLineArgumentsTest.cpp
        oatpp/base/CommandLineArgumentsTest.hpp
        oatpp/base/LogTest.cpp
        oatpp/base/LogTest.hpp
        oatpp/data/buffer/ProcessorTest.cpp
        oatpp/data/buffer/ProcessorTest.hpp
        oatpp/data/mapping/ObjectRemapperTest.cpp
        oatpp/data/mapping/ObjectRemapperTest.hpp
        oatpp/data/mapping/ObjectToTreeMapperTest.cpp
        oatpp/data/mapping/ObjectToTreeMapperTest.hpp
        oatpp/data/mapping/TreeTest.cpp
        oatpp/data/mapping/TreeTest.hpp
        oatpp/data/mapping/TreeToObjectMapperTest.cpp
        oatpp/data/mapping/TreeToObjectMapperTest.hpp
        oatpp/data/mapping/TypeResolverTest.cpp
        oatpp/data/mapping/TypeResolverTest.hpp
        oatpp/data/resource/InMemoryDataTest.cpp
        oatpp/data/resource/InMemoryDataTest.hpp
        oatpp/data/share/LazyStringMapTest.cpp
        oatpp/data/share/LazyStringMapTest.hpp
        oatpp/data/share/MemoryLabelTest.cpp
        oatpp/data/share/MemoryLabelTest.hpp
        oatpp/data/share/StringTemplateTest.cpp
        oatpp/data/share/StringTemplateTest.hpp
        oatpp/data/stream/BufferStreamTest.cpp
        oatpp/data/stream/BufferStreamTest.hpp
        oatpp/data/type/AnyTest.cpp
        oatpp/data/type/AnyTest.hpp
        oatpp/data/type/EnumTest.cpp
        oatpp/data/type/EnumTest.hpp
        oatpp/data/type/InterpretationTest.cpp
        oatpp/data/type/InterpretationTest.hpp
        oatpp/data/type/ListTest.cpp
        oatpp/data/type/ListTest.hpp
        oatpp/data/type/ObjectTest.cpp
        oatpp/data/type/ObjectTest.hpp
        oatpp/data/type/ObjectWrapperTest.cpp
        oatpp/data/type/ObjectWrapperTest.hpp
        oatpp/data/type/PairListTest.cpp
        oatpp/data/type/PairListTest.hpp
        oatpp/data/type/PrimitiveTest.cpp
        oatpp/data/type/PrimitiveTest.hpp
        oatpp/data/type/StringTest.cpp
        oatpp/data/type/StringTest.hpp
        oatpp/data/type/TypeTest.cpp
        oatpp/data/type/TypeTest.hpp
        oatpp/data/type/UnorderedMapTest.cpp
        oatpp/data/type/UnorderedMapTest.hpp
        oatpp/data/type/UnorderedSetTest.cpp
        oatpp/data/type/UnorderedSetTest.hpp
        oatpp/data/type/VectorTest.cpp
        oatpp/data/type/VectorTest.hpp
        oatpp/encoding/Base64Test.cpp
        oatpp/encoding/Base64Test.hpp
        oatpp/encoding/HexTest.cpp
        oatpp/encoding/HexTest.hpp
        oatpp/encoding/UnicodeTest.cpp
        oatpp/encoding/UnicodeTest.hpp
        oatpp/encoding/UrlTest.cpp
        oatpp/encoding/UrlTest.hpp
        oatpp/json/BooleanTest.cpp
        oatpp/json/BooleanTest.hpp
        oatpp/json/DeserializerTest.cpp
        oatpp/json/DeserializerTest.hpp
        oatpp/json/DTOMapperPerfTest.cpp
        oatpp/json/DTOMapperPerfTest.hpp
        oatpp/json/DTOMapperTest.cpp
        oatpp/json/DTOMapperTest.hpp
        oatpp/json/EnumTest.cpp
        oatpp/json/EnumTest.hpp
        oatpp/json/UnorderedSetTest.cpp
        oatpp/json/UnorderedSetTest.hpp
        oatpp/network/ConnectionPoolTest.cpp
        oatpp/network/ConnectionPoolTest.hpp
        oatpp/network/UrlTest.cpp
        oatpp/network/UrlTest.hpp
        oatpp/network/monitor/ConnectionMonitorTest.cpp
        oatpp/network/monitor/ConnectionMonitorTest.hpp
        oatpp/network/virtual_/InterfaceTest.cpp
        oatpp/network/virtual_/InterfaceTest.hpp
        oatpp/network/virtual_/PipeTest.cpp
        oatpp/network/virtual_/PipeTest.hpp
        oatpp/provider/PoolTemplateTest.cpp
        oatpp/provider/PoolTemplateTest.hpp
        oatpp/provider/PoolTest.cpp
        oatpp/provider/PoolTest.hpp
        oatpp/utils/parser/CaretTest.cpp
        oatpp/utils/parser/CaretTest.hpp
        oatpp/web/ClientRetryTest.cpp
        oatpp/web/ClientRetryTest.hpp
        oatpp/web/FullAsyncClientTest.cpp
        oatpp/web/FullAsyncClientTest.hpp
        oatpp/web/FullAsyncTest.cpp
        oatpp/web/FullAsyncTest.hpp
        oatpp/web/FullTest.cpp
        oatpp/web/FullTest.hpp
        oatpp/web/PipelineAsyncTest.cpp
        oatpp/web/PipelineAsyncTest.hpp
        oatpp/web/PipelineTest.cpp
        oatpp/web/PipelineTest.hpp
        oatpp/web/app/BasicAuthorizationController.hpp
        oatpp/web/app/BearerAuthorizationController.hpp
        oatpp/web/app/Client.hpp
        oatpp/web/app/Controller.hpp
        oatpp/web/app/ControllerAsync.hpp
        oatpp/web/app/ControllerWithErrorHandler.hpp
        oatpp/web/app/ControllerWithInterceptors.hpp
        oatpp/web/app/ControllerWithInterceptorsAsync.hpp
        oatpp/web/app/DTOs.hpp
        oatpp/web/mime/multipart/StatefulParserTest.cpp
        oatpp/web/mime/multipart/StatefulParserTest.hpp
        oatpp/web/mime/ContentMappersTest.cpp
        oatpp/web/mime/ContentMappersTest.hpp
        oatpp/web/protocol/http/encoding/ChunkedTest.cpp
        oatpp/web/protocol/http/encoding/ChunkedTest.hpp
        oatpp/web/server/HttpRouterTest.cpp
        oatpp/web/server/HttpRouterTest.hpp
        oatpp/web/server/ServerStopTest.cpp
        oatpp/web/server/ServerStopTest.hpp
        oatpp/web/server/api/ApiControllerTest.cpp
        oatpp/web/server/api/ApiControllerTest.hpp
        oatpp/web/server/handler/AuthorizationHandlerTest.cpp
        oatpp/web/server/handler/AuthorizationHandlerTest.hpp
        oatpp/AllTestsMain.cpp
        oatpp/LoggerTest.cpp
        oatpp/LoggerTest.hpp
)
set_target_source_groups(oatppAllTests STRIP_PREFIX "oatpp")

target_link_libraries(oatppAllTests PRIVATE oatpp PRIVATE oatpp-test)

set_target_properties(oatppAllTests PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    VERSION "${oatpp_VERSION}"
    SOVERSION "${oatpp_VERSION_MAJOR}.${oatpp_VERSION_MINOR}"
)
if (MSVC)
    target_compile_options(oatppAllTests PRIVATE /permissive-)
endif()

target_include_directories(oatppAllTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_test(oatppAllTests oatppAllTests)
