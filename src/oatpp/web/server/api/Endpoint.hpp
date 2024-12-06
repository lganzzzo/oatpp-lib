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

#ifndef oatpp_web_server_api_Endpoint_hpp
#define oatpp_web_server_api_Endpoint_hpp

#include "oatpp/web/server/HttpRequestHandler.hpp"

#include <list>
#include <unordered_map>
#include <functional>

namespace oatpp { namespace web { namespace server { namespace api {

/**
 * Endpoint - class which holds information about endpoint.
 * It holds API documentation info, and pointer to RequestHandler
 */
class Endpoint : public oatpp::base::Countable {
public:

  /**
   * Convenience typedef for &id:oatpp::web::server::HttpRequestHandler;.
   */
  typedef oatpp::web::server::HttpRequestHandler RequestHandler;
public:

  /**
   * Info holds API documentation information about endpoint
   */
  class Info : public oatpp::base::Countable {
  public:

    /**
     * Param holds info about parameter
     */
    struct Param {
      
      Param();
      Param(const oatpp::String& pName, oatpp::data::type::Type* pType);

      oatpp::String name;
      oatpp::data::type::Type* type;

      oatpp::String description;
      oatpp::Boolean required = true;
      oatpp::Boolean deprecated = false;
      oatpp::Boolean allowEmptyValue;
      std::list<std::pair<oatpp::String, oatpp::Any>> examples;

      Param& addExample(const oatpp::String& title, const oatpp::Any& example) {
        examples.push_back({title, example});
        return *this;
      }

    };

    /**
     * Parameters container
     */
    class Params {
    private:
      std::list<oatpp::String> m_order;
      std::unordered_map<oatpp::String, Param> m_params;
    public:

      const std::list<oatpp::String>& getOrder() const;

      /**
       * Add parameter name to list order
       * @param name
       * @return new or existing parameter
       */
      Param& add(const oatpp::String& name, oatpp::data::type::Type* type);

      /**
       * Add parameter name to list order
       * @tparam T
       * @param aname
       * @return new or existing parameter
       */
      template<class T>
      Param& add(const oatpp::String& aname) {
        return add(aname, T::Class::getType());
      }

      /**
       * Get or add param by name
       * @param name
       * @return
       */
      Param& operator [](const oatpp::String& name);

    };

    /**
     * Hints about the response (content-type, schema, description, ...)
     */
    struct ContentHints {
      oatpp::String contentType;
      oatpp::data::type::Type* schema;
      oatpp::String description;
      std::list<std::pair<oatpp::String, oatpp::Any>> examples;

      ContentHints& addExample(const oatpp::String& title, const oatpp::Any& example) {
        examples.push_back({title, example});
        return *this;
      }
    };
    
  public:

    /**
     * Constructor;
     */
    Info();

    /**
     * Create shared Info.
     * @return
     */
    static std::shared_ptr<Info> createShared();

    /**
     * Endpoint name.
     */
    oatpp::String name;

    /**
     * Endpoint summary.
     */
    oatpp::String summary;

    /**
     * Endpoint description.
     */
    oatpp::String description;

    /**
     * Endpoint path.
     */
    oatpp::String path;

    /**
     * HTTP method.
     */
    oatpp::String method;

    /**
     * Authorization.
     */
    oatpp::String authorization;

    /**
     * Hide endpoint from the documentation.
     */
    oatpp::Boolean hide;

    /**
     * Is endpoint deprecated.
     */
    oatpp::Boolean deprecated;

    /**
     * Tags to group endpoints in the documentation.
     */
    std::list<oatpp::String> tags;

    /**
     * Body info.
     */
    Param body;

    /**
     * Body content type.
     */
    oatpp::String bodyContentType;

    /**
     * Consumes.
     */
    std::list<ContentHints> consumes;

    /**
     * Security Requirements
     */
    std::unordered_map<oatpp::String, std::shared_ptr<std::list<oatpp::String>>> securityRequirements;

    /**
     * Headers.
     */
    Params headers;

    /**
     * Path variables.
     */
    Params pathParams;

    /**
     * Query params.
     */
    Params queryParams;
    
    /**
     *  ResponseCode to {ContentType, Type} mapping.
     *  Example responses[Status::CODE_200] = {"application/json", MyDto::ObjectWrapper::Class::getType()};
     */
    std::unordered_map<oatpp::web::protocol::http::Status, ContentHints> responses;
    
    oatpp::String toString();

    /**
     * Add "consumes" info to endpoint.
     * @tparam T
     * @param contentType
     */
    template<class Wrapper>
    ContentHints& addConsumes(const oatpp::String& contentType, const oatpp::String& desc = oatpp::String()) {
      consumes.push_back({contentType, Wrapper::Class::getType(), desc});
      return consumes.back();
    }

    /**
     * Add response info to endpoint
     * @tparam Wrapper
     * @param status
     * @param contentType
     * @param responseDescription
     */
    template<class Wrapper>
    ContentHints& addResponse(const oatpp::web::protocol::http::Status& status, const oatpp::String& contentType, const oatpp::String& responseDescription = oatpp::String()) {
      auto& hint = responses[status];
      hint.contentType = contentType;
      hint.description = responseDescription.get() == nullptr ? status.description : responseDescription;
      hint.schema = Wrapper::Class::getType();
      return hint;
    }

    /**
     * Add response info with no message-body to endpoint
     * @param status
     * @param responseDescription
     */
    ContentHints& addResponse(const oatpp::web::protocol::http::Status& status, const oatpp::String& responseDescription = oatpp::String()) {
      auto& hint = responses[status];
      hint.description = responseDescription.get() == nullptr ? status.description : responseDescription;
      return hint;
    }

    /**
     * Add security requirement.
     * @param requirement
     * @param scopes
     */
    void addSecurityRequirement(const oatpp::String &requirement, const std::shared_ptr<std::list<oatpp::String>> &scopes = nullptr) {
      securityRequirements[requirement] = scopes;
    }

    /**
     * Add tag.
     * @param tag
     */
    void addTag(const oatpp::String& tag) {
      tags.push_back(tag);
    }

  };
public:

  Endpoint(const std::shared_ptr<RequestHandler>& pHandler, const std::function<std::shared_ptr<Endpoint::Info>()>& infoBuilder);
  
  static std::shared_ptr<Endpoint> createShared(const std::shared_ptr<RequestHandler>& handler,
                                                const std::function<std::shared_ptr<Endpoint::Info>()>& infoBuilder);
  
  const std::shared_ptr<RequestHandler> handler;

  std::shared_ptr<Info> info();

private:
  std::shared_ptr<Info> m_info;
  std::function<std::shared_ptr<Endpoint::Info>()> m_infoBuilder;
  
};

/**
 * Collection of endpoints.
 */
struct Endpoints {

  std::list<std::shared_ptr<Endpoint>> list;

  void append(const std::list<std::shared_ptr<Endpoint>>& endpoints);
  void append(const Endpoints& endpoints);
  void append(const std::shared_ptr<Endpoint>& endpoint);

};
  
}}}}

#endif /* oatpp_web_server_api_Endpoint_hpp */
