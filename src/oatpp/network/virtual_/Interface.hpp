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

#ifndef oatpp_network_virtual__Interface_hpp
#define oatpp_network_virtual__Interface_hpp

#include "./Socket.hpp"

#include <list>
#include <unordered_map>
#include <chrono>

namespace oatpp { namespace network { namespace virtual_ {

/**
 * "Virtual" Interface provides functionality for accepting "virtual" connections.
 * "virtual" connection is represented by &id:oatpp::network::virtual_::Socket;.
 */
class Interface : public oatpp::base::Countable {
private:
  static std::recursive_mutex m_registryMutex;
  static std::unordered_map<oatpp::String, std::weak_ptr<Interface>> m_registry;
private:
  static void registerInterface(const std::shared_ptr<Interface>& _interface);
  static void unregisterInterface(const oatpp::String& name);
public:

  /**
   * `ListenerLock` is the Lock object, which represents interface binding ownership.
   */
  class ListenerLock {
    friend Interface;
  private:
    Interface* m_interface;
  private:
    ListenerLock(Interface* _interface);
  public:
    ~ListenerLock();
  };

public:

  /**
   * "Future" for &id:oatpp::network::virtual_::Socket;.
   */
  class ConnectionSubmission {
    friend Interface;
  private:
    std::shared_ptr<Socket> m_socket;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_valid;
  private:
    void invalidate();
  public:

    /**
     * Constructor.
     */
    ConnectionSubmission(bool valid) : m_valid(valid) {}

    /**
     * Set socket to be returned in call to &l:Interface::ConnectionSubmission::getSocket ();/&l:Interface::ConnectionSubmission::getSocketNonBlocking ();.
     * @param socket - &id:oatpp::network::virtual_::Socket;.
     */
    void setSocket(const std::shared_ptr<Socket>& socket);

    /**
     * Block and wait for socket.
     * @return - `std::shared_ptr` to &id:oatpp::network::virtual_::Socket;.
     */
    std::shared_ptr<Socket> getSocket();

    /**
     * Check if socket already available.
     * User should repeat call if `(!socket && isPending())`.
     * @return - `std::shared_ptr` to &id:oatpp::network::virtual_::Socket;.
     */
    std::shared_ptr<Socket> getSocketNonBlocking();

    /**
     * Check if submission is still valid.
     * @return - `true` if still valid.
     */
    bool isValid();
    
  };

private:
  std::shared_ptr<Socket> acceptSubmission(const std::shared_ptr<ConnectionSubmission>& submission);
  void unbindListener(ListenerLock* listenerLock);
private:
  oatpp::String m_name;
  std::atomic<ListenerLock*> m_listenerLock;
  std::mutex m_listenerMutex;
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::list<std::shared_ptr<ConnectionSubmission>> m_submissions;
private:

  Interface(const oatpp::String& name);

  Interface(const Interface& other) = delete;
  Interface(Interface&& other) = delete;
  Interface& operator=(const Interface&) = delete;
  Interface& operator=(Interface&&) = delete;

public:

  /**
   * Destructor.
   */
  ~Interface() override;

  /**
   * Obtain interface for given name.
   * @param name - name of the interface.
   * @return - `std::shared_ptr` to Interface.
   */
  static std::shared_ptr<Interface> obtainShared(const oatpp::String& name);

  /**
   * Aquire &l:Interface::ListenerLock ;.
   * @return - `std::shared_ptr` to &l:Interface::ListenerLock ;.
   */
  std::shared_ptr<ListenerLock> bind();

  /**
   * Connect to interface.
   * @return - &l:Interface::ConnectionSubmission;.
   */
  std::shared_ptr<ConnectionSubmission> connect();

  /**
   * Connect to interface.
   * @return - &l:Interface::ConnectionSubmission; on success. Empty `std::shared_ptr` on failure.
   */
  std::shared_ptr<ConnectionSubmission> connectNonBlocking();

  /**
   * Block and wait for incloming connection.
   * @param waitingHandle - reference to a boolean variable.
   * User may set waitingHandle = false and call &l:Interface::notifyAcceptors (); in order to break waiting loop. and exit accept() method.
   * @param timeout
   * @return - `std::shared_ptr` to &id:oatpp::network::virtual_::Socket;.
   */
  std::shared_ptr<Socket> accept(const bool& waitingHandle = true,
                                 const std::chrono::duration<v_int64, std::micro>& timeout = std::chrono::minutes (10));

  /**
   * Check if incoming connection is available. NonBlocking.
   * @return - `std::shared_ptr` to &id:oatpp::network::virtual_::Socket; if available.
   * Empty `std::shared_ptr` if no incoming connection is available at the moment.
   */
  std::shared_ptr<Socket> acceptNonBlocking();

  /**
   * Drop all waiting connections.
   */
  void dropAllConnection();

  /**
   * Notify all threads that are waiting on accept().
   * Those threads that have waitingHandle changed to false will be unblocked.
   */
  void notifyAcceptors();

  /**
   * Get interface name.
   * @return - &id:oatpp::String;.
   */
  oatpp::String getName() {
    return m_name;
  }

  oatpp::async::CoroutineWaitList m_serverWaitList, m_clientWaitList;

};
  
}}}

#endif /* oatpp_network_virtual__Interface_hpp */
