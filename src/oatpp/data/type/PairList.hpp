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

#ifndef oatpp_data_type_PairList_hpp
#define oatpp_data_type_PairList_hpp

#include "./Map.hpp"
#include "./Type.hpp"

#include <list>
#include <initializer_list>
#include <utility>

namespace oatpp { namespace data { namespace type {

namespace __class {

  /**
   * Abstract PairList class.
   */
  class AbstractPairList {
  public:
    /**
     * Class id.
     */
    static const ClassId CLASS_ID;
  };

  template<class Key, class Value>
  class PairList;

}

/**
 * `ObjectWrapper` over `std::list<std::pair<Key, Value>>`
 * @tparam Key - Key `ObjectWrapper` type.
 * @tparam Value - Value `ObjectWrapper` type.
 * @tparam C - Class.
 */
template<class Key, class Value, class C>
class PairListObjectWrapper : public type::ObjectWrapper<std::list<std::pair<Key, Value>>, C> {
public:
  typedef std::list<std::pair<Key, Value>> TemplateObjectType;
  typedef C TemplateObjectClass;
public:

  OATPP_DEFINE_OBJECT_WRAPPER_DEFAULTS(PairListObjectWrapper, TemplateObjectType, TemplateObjectClass)

  PairListObjectWrapper(std::initializer_list<std::pair<Key, Value>> ilist)
    : type::ObjectWrapper<TemplateObjectType, TemplateObjectClass>(std::make_shared<TemplateObjectType>(ilist))
  {}

  static PairListObjectWrapper createShared() {
    return std::make_shared<TemplateObjectType>();
  }

  PairListObjectWrapper& operator = (std::initializer_list<std::pair<Key, Value>> ilist) {
    this->m_ptr = std::make_shared<TemplateObjectType>(ilist);
    return *this;
  }

  std::pair<Key, Value>& operator[] (v_buff_usize index) const {
    auto it = this->m_ptr->begin();
    std::advance(it, index);
    return *it;
  }
  
  Value& operator[] (const Key& key) const {
    auto& list = *(this->m_ptr.get());
    auto it = list.begin();
    while(it != list.end()) {
      if(it->first == key) {
        return it->second;
      }
      it ++;
    }
    list.push_back({key, nullptr});
    return list.back().second;
  }

  Value getValueByKey(const Key& key, const Value& defValue = nullptr) const {
    auto& list = *(this->m_ptr.get());
    auto it = list.begin();
    while(it != list.end()) {
      if(it->first == key) {
        return it->second;
      }
      it ++;
    }
    return defValue;
  }

  TemplateObjectType& operator*() const {
    return this->m_ptr.operator*();
  }

};

/**
 * Mapping-Enables PairList<Key, Value>. See &l:PairListObjectWrapper;.
 */
template<class Key, class Value>
using PairList = PairListObjectWrapper<Key, Value, __class::PairList<Key, Value>>;

namespace __class {

template<class Key, class Value>
class PairList : public AbstractPairList {
private:

  static Type createType() {
    Type::Info info;
    info.params.push_back(Key::Class::getType());
    info.params.push_back(Value::Class::getType());
    static std::unique_ptr<typename __class::StandardMap<std::list<std::pair<Key, Value>>, Key, Value, PairList>::PolymorphicDispatcher> disp(new typename __class::StandardMap<std::list<std::pair<Key, Value>>, Key, Value, PairList>::PolymorphicDispatcher());
    info.polymorphicDispatcher = disp.get();
    info.isMap = true;
    return Type(__class::AbstractPairList::CLASS_ID, info);
  }

public:

  static Type* getType() {
    static Type type = createType();
    return &type;
  }

};

}

}}}

#endif // oatpp_data_type_PairList_hpp
