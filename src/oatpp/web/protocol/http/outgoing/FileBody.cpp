/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2024-present, MrBesen
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

#include "FileBody.hpp"

#if !defined(WIN32) && !defined(_WIN32)
#include <fcntl.h> // open
#include <sys/mman.h> // mmap, madvise
#include <unistd.h> // lseek
#endif

namespace oatpp { namespace web { namespace protocol { namespace http { namespace outgoing {

FileBody::FileBody(const oatpp::String &filepath, const data::share::StringKeyLabel &contentType)
  : m_filepath(filepath)
#if defined(WIN32) || defined(_WIN32)
  , m_buffer(oatpp::String::loadFromFile(filepath->c_str()))
#else
  , m_ptr(nullptr)
  , m_initial_size(-1)
#endif
  , m_contentType(contentType)
  , m_inlineData(nullptr, 0)
{
#if defined(WIN32) || defined(_WIN32)
  m_inlineData = InlineReadData(reinterpret_cast<void*>(m_buffer->data()), static_cast<v_buff_size>(m_buffer->size()));
#else
  int fd = ::open(m_filepath->c_str(), O_RDONLY | O_CLOEXEC);
  if(fd == -1) {
    return;
  }

  // map file
  m_initial_size = ::lseek64(fd, 0, SEEK_END);
  m_ptr = ::mmap(nullptr, m_initial_size, PROT_READ, MAP_SHARED, fd, 0);
  ::close(fd);
  if(m_ptr == MAP_FAILED) {
    return;
  }

  // set some hints for the kernel
  ::madvise(m_ptr, m_initial_size, MADV_SEQUENTIAL);
  ::madvise(m_ptr, m_initial_size, MADV_WILLNEED);

  m_inlineData = InlineReadData(m_ptr, m_initial_size);
#endif
}

FileBody::~FileBody() {
#if !defined(WIN32) && !defined(_WIN32)
  // remove mapping
  ::munmap(m_ptr, m_initial_size);
#endif
}

std::shared_ptr<FileBody> FileBody::createShared(const oatpp::String &filepath,
                                                     const data::share::StringKeyLabel &contentType) {
  return std::make_shared<FileBody>(filepath, contentType);
}

v_io_size FileBody::read(void *buffer, v_buff_size count, async::Action &action) {

  (void) action;

  v_buff_size desiredToRead = m_inlineData.bytesLeft;

  if (desiredToRead > 0) {

    if (desiredToRead > count) {
      desiredToRead = count;
    }

    std::memcpy(buffer, m_inlineData.currBufferPtr, static_cast<size_t>(desiredToRead));
    m_inlineData.inc(desiredToRead);

    return desiredToRead;
  }

  return 0;
}

void FileBody::declareHeaders(Headers &headers) {
  if (m_contentType) {
    headers.putIfNotExists(Header::CONTENT_TYPE, m_contentType);
  }
}

p_char8 FileBody::getKnownData() {
#if defined(WIN32) || defined(_WIN32)
  return reinterpret_cast<p_char8>(m_buffer->data());
#else
  return reinterpret_cast<p_char8>(m_ptr);
#endif
}

v_int64 FileBody::getKnownSize() {
#if defined(WIN32) || defined(_WIN32)
  return static_cast<v_int64>(m_buffer->size());
#else
  return static_cast<v_int64>(m_initial_size);
#endif
}

}}}}}
