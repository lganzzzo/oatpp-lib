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

#ifndef oatpp_data_type_List_hpp
#define oatpp_data_type_List_hpp

#include "./Collection.hpp"
#include "./Type.hpp"

#include <list>
#include <initializer_list>

namespace oatpp { namespace data { namespace type {

namespace __class {

  /**
   * Abstract list class.
   */
  class AbstractList {
  public:

    /**
     * Class Id.
     */
    static const ClassId CLASS_ID;

  };

  template<class T>
  class List;

}

/**
 * `ObjectWrapper` over `std::list<T>`
 * @tparam T - Item `ObjectWrapper` type.
 * @tparam C - Class.
 */
template<class T, class C>
class ListObjectWrapper : public type::ObjectWrapper<std::list<T>, C> {
public:
  typedef std::list<T> TemplateObjectType;
  typedef C TemplateObjectClass;
public:

  OATPP_DEFINE_OBJECT_WRAPPER_DEFAULTS(ListObjectWrapper, TemplateObjectType, TemplateObjectClass)

  ListObjectWrapper(std::initializer_list<T> ilist)
    : type::ObjectWrapper<TemplateObjectType, TemplateObjectClass>(std::make_shared<TemplateObjectType>(ilist))
  {}

  static ListObjectWrapper createShared() {
    return std::make_shared<TemplateObjectType>();
  }

  ListObjectWrapper& operator = (std::initializer_list<T> ilist) {
    this->m_ptr = std::make_shared<TemplateObjectType>(ilist);
    return *this;
  }

  T& operator[] (v_buff_usize index) const {
    auto it = this->m_ptr->begin();
    std::advance(it, index);
    return *it;
  }

  TemplateObjectType& operator*() const {
    return this->m_ptr.operator*();
  }

};

/**
 * Mapping-Enabled List. See - &l:ListObjectWrapper;.
 */
template<class T>
using List = ListObjectWrapper<T, __class::List<T>>;

typedef List<Void> AbstractList;

namespace __class {

  template<class T>
  class List : public AbstractList {
  private:

    static Type createType() {
      Type::Info info;
      info.params.push_back(T::Class::getType());
      static std::unique_ptr<typename StandardCollection<std::list<T>, T, List>::PolymorphicDispatcher> disp(new typename StandardCollection<std::list<T>, T, List>::PolymorphicDispatcher());
      info.polymorphicDispatcher = disp.get();
      info.isCollection = true;
      return Type(__class::AbstractList::CLASS_ID, info);
    }

  public:

    static Type* getType() {
      static Type type = createType();
      return &type;
    }

  };

}

}}}

#endif // oatpp_data_type_List_hpp
