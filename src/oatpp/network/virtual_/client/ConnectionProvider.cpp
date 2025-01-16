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

#include "ConnectionProvider.hpp"
#include "oatpp/core/async/ConditionVariable.hpp"

namespace oatpp { namespace network { namespace virtual_ { namespace client {

void ConnectionProvider::ConnectionInvalidator::invalidate(const std::shared_ptr<data::stream::IOStream>& connection) {
  auto socket = std::static_pointer_cast<Socket>(connection);
  socket->close();
}

ConnectionProvider::ConnectionProvider(const std::shared_ptr<virtual_::Interface>& _interface)
  : m_invalidator(std::make_shared<ConnectionInvalidator>())
  , m_interface(_interface)
  , m_maxAvailableToRead(-1)
  , m_maxAvailableToWrite(-1)
{
  setProperty(PROPERTY_HOST, m_interface->getName());
  setProperty(PROPERTY_PORT, "0");
}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<virtual_::Interface>& _interface) {
  return std::make_shared<ConnectionProvider>(_interface);
}

void ConnectionProvider::stop() {

}

provider::ResourceHandle<data::stream::IOStream> ConnectionProvider::get() {
  auto submission = m_interface->connect();
  if(submission->isValid()) {
    auto socket = submission->getSocket();
    if (socket) {
      socket->setOutputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
      socket->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
      socket->setMaxAvailableToReadWrtie(m_maxAvailableToRead, m_maxAvailableToWrite);
      return provider::ResourceHandle<data::stream::IOStream>(socket, m_invalidator);
    }
  }
  throw std::runtime_error("[oatpp::network::virtual_::client::getConnection()]: Error. Can't connect. " + *m_interface->getName());
}
  
oatpp::async::CoroutineStarterForResult<const provider::ResourceHandle<data::stream::IOStream>&>
ConnectionProvider::getAsync() {
  
  class ConnectCoroutine : public oatpp::async::CoroutineWithResult<ConnectCoroutine, const provider::ResourceHandle<oatpp::data::stream::IOStream>&> {
  private:
    std::shared_ptr<ConnectionInvalidator> m_invalidator;
    std::shared_ptr<virtual_::Interface> m_interface;
    v_io_size m_maxAvailableToRead;
    v_io_size m_maxAvailableToWrite;
    std::shared_ptr<virtual_::Interface::ConnectionSubmission> m_submission;
    std::shared_ptr<Socket> m_socket;
  public:
    
    ConnectCoroutine(const std::shared_ptr<ConnectionInvalidator>& invalidator,
                     const std::shared_ptr<virtual_::Interface>& _interface,
                     v_io_size maxAvailableToRead,
                     v_io_size maxAvailableToWrite)
      : m_invalidator(invalidator)
      , m_interface(_interface)
      , m_maxAvailableToRead(maxAvailableToRead)
      , m_maxAvailableToWrite(maxAvailableToWrite)
    {}
    
    Action act() override {
      m_submission = m_interface->connectNonBlocking();
      if(m_submission){
        m_interface->m_serverWaitList.notifyFirst();
        if (m_submission->isValid()) return yieldTo(&ConnectCoroutine::obtainSocket);
        return error<Error>("[oatpp::network::virtual_::client::ConnectionProvider::getConnectionAsync()]: Error. submission not valid.");
      }
      return repeat();
    }

    Action obtainSocket() {
      auto socket = m_submission->getSocketNonBlocking();
      if(socket) {
        socket->setOutputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);
        socket->setInputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);
        socket->setMaxAvailableToReadWrtie(m_maxAvailableToRead, m_maxAvailableToWrite);
        return _return(provider::ResourceHandle<data::stream::IOStream>(socket, m_invalidator));
      }
      return Action::createWaitListAction(
        &m_interface->m_clientWaitList, 
        std::chrono::system_clock::now()+std::chrono::milliseconds(100));
    }
    
  };
  
  return ConnectCoroutine::startForResult(m_invalidator, m_interface, m_maxAvailableToRead, m_maxAvailableToWrite);
  
}
  
}}}}
