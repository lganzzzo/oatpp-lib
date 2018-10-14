/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
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

#ifndef oatpp_web_server_HttpRouter_hpp
#define oatpp_web_server_HttpRouter_hpp

#include "oatpp/web/protocol/http/outgoing/Response.hpp"
#include "oatpp/web/protocol/http/incoming/Request.hpp"

#include "oatpp/web/url/mapping/Router.hpp"

namespace oatpp { namespace web { namespace server {
  
class HttpRouter : public oatpp::base::Controllable {
public:
  typedef oatpp::web::url::mapping::Router<
    std::shared_ptr<oatpp::web::protocol::http::incoming::Request>,
    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response>
  > BranchRouter;
  typedef BranchRouter::UrlSubscriber Subscriber;
  typedef oatpp::collection::ListMap<oatpp::String, std::shared_ptr<BranchRouter>> BranchMap;
protected:
  std::shared_ptr<BranchMap> m_branchMap;
protected:
  
  const std::shared_ptr<BranchRouter>& getBranch(const oatpp::String& name){
    auto entry = m_branchMap->find(name);
    if(entry == nullptr){
      auto branch = BranchRouter::createShared();
      entry = m_branchMap->put(name, branch);
    }
    return entry->getValue();
  }
  
public:
  HttpRouter()
    : m_branchMap(BranchMap::createShared())
  {}
public:
  
  static std::shared_ptr<HttpRouter> createShared() {
    return std::make_shared<HttpRouter>();
  }
  
  void addSubscriber(const oatpp::String& method,
                     const oatpp::String& urlPattern,
                     const std::shared_ptr<Subscriber>& subscriber) {
    getBranch(method)->addSubscriber(urlPattern, subscriber);
  }
  
  BranchRouter::Route getRoute(const oatpp::String& method,
                               const oatpp::String& url){
    const std::shared_ptr<BranchRouter>& branch = m_branchMap->get(method, nullptr);
    if(branch){
      return branch->getRoute(url);
    }
    return BranchRouter::Route(nullptr, nullptr);
  }
  
  void logRouterMappings() {
    auto curr = m_branchMap->getFirstEntry();
    while (curr != nullptr) {
      curr->getValue()->logRouterMappings();
      curr = curr->getNext();
    }
  }
  
};
  
}}}

#endif /* oatpp_web_server_HttpRouter_hpp */
