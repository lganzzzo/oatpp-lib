/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *                         Benedikt-Alexander Mokroß <bam@icognize.de>
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

// Defaults

/**
 * Codegen macro to be used in classes extending &id:oatpp::data::type::Object; to generate required fields/methods/constructors for DTO object.
 * @param TYPE_NAME - name of the DTO class.
 * @param TYPE_EXTEND - name of the parent DTO class. If DTO extends &id:oatpp::data::type::Object; TYPE_EXETENDS should be `Object`.
 */
#define DTO_INIT(TYPE_NAME, TYPE_EXTEND) \
  template<class __Z__T__PARAM> \
  friend class oatpp::data::type::__class::Object; \
public: \
  typedef TYPE_NAME Z__CLASS; \
  typedef TYPE_EXTEND Z__CLASS_EXTENDED; \
  typedef oatpp::data::type::DTOWrapper<Z__CLASS> Wrapper; \
private: \
\
  static const oatpp::Type* getParentType() { \
    return oatpp::Object<Z__CLASS_EXTENDED>::Class::getType(); \
  } \
\
  static const char* Z__CLASS_TYPE_NAME() { \
    return #TYPE_NAME; \
  } \
\
  static oatpp::data::type::BaseObject::Properties* Z__CLASS_GET_FIELDS_MAP(){ \
    static oatpp::data::type::BaseObject::Properties map = oatpp::data::type::BaseObject::Properties(); \
    return &map; \
  } \
\
public: \
\
  template<typename ... Args> \
  static Wrapper createShared(Args... args){ \
    return Wrapper(std::make_shared<Z__CLASS>(args...), Wrapper::Class::getType()); \
  }

// Fields

#define OATPP_MACRO_DTO_FIELD_1(TYPE, NAME) \
\
static v_int64 Z__PROPERTY_OFFSET_##NAME() { \
  char buffer[sizeof(Z__CLASS)]; \
  auto obj = static_cast<Z__CLASS*>(reinterpret_cast<void*>(buffer)); \
  auto ptr = &obj->NAME; \
  return reinterpret_cast<v_int64>(ptr) - reinterpret_cast<v_int64>(buffer); \
} \
\
static oatpp::data::type::BaseObject::Property* Z__PROPERTY_SINGLETON_##NAME() { \
  static std::unique_ptr<oatpp::data::type::BaseObject::Property> property( \
      new oatpp::data::type::BaseObject::Property(Z__PROPERTY_OFFSET_##NAME(), \
                                                  #NAME, \
                                                  #NAME, \
                                                  TYPE::Class::getType())); \
  return property.get(); \
} \
\
static bool Z__PROPERTY_INIT_##NAME(... /* default initializer for all cases */) { \
  Z__CLASS_GET_FIELDS_MAP()->pushBack(Z__PROPERTY_SINGLETON_##NAME()); \
  return true; \
} \
\
static TYPE Z__PROPERTY_INITIALIZER_PROXY_##NAME() { \
  static bool initialized = Z__PROPERTY_INIT_##NAME(1 /* init info if found */,           \
                                                    1 /* init type selector if found */); \
  (void)initialized; \
  return TYPE(); \
} \
\
TYPE NAME = Z__PROPERTY_INITIALIZER_PROXY_##NAME()

#define OATPP_MACRO_DTO_FIELD_2(TYPE, NAME, QUALIFIER) \
\
static v_int64 Z__PROPERTY_OFFSET_##NAME() { \
  char buffer[sizeof(Z__CLASS)]; \
  auto obj = static_cast<Z__CLASS*>(reinterpret_cast<void*>(buffer)); \
  auto ptr = &obj->NAME; \
  return reinterpret_cast<v_int64>(ptr) - reinterpret_cast<v_int64>(buffer); \
} \
\
static oatpp::data::type::BaseObject::Property* Z__PROPERTY_SINGLETON_##NAME() { \
  static std::unique_ptr<oatpp::data::type::BaseObject::Property> property( \
      new oatpp::data::type::BaseObject::Property(Z__PROPERTY_OFFSET_##NAME(), \
                                                  QUALIFIER, \
                                                  #NAME, \
                                                  TYPE::Class::getType())); \
  return property.get(); \
} \
\
static bool Z__PROPERTY_INIT_##NAME(... /* default initializer for all cases */) { \
  Z__CLASS_GET_FIELDS_MAP()->pushBack(Z__PROPERTY_SINGLETON_##NAME()); \
  return true; \
} \
\
static TYPE Z__PROPERTY_INITIALIZER_PROXY_##NAME() { \
  static bool initialized = Z__PROPERTY_INIT_##NAME(1 /* init info if found */,           \
                                                    1 /* init type selector if found */); \
  (void)initialized; \
  return TYPE(); \
} \
\
TYPE NAME = Z__PROPERTY_INITIALIZER_PROXY_##NAME()

/**
 * Codegen macro to generate fields of DTO object.
 * @param TYPE - type of the field.
 * @param NAME - name of the field.
 * @param QUALIFIER_NAME - additional (optional) field to specify serialized name of the field. If not specified it will be same as NAME.
 */
#define DTO_FIELD(TYPE, ...) \
OATPP_MACRO_EXPAND(OATPP_MACRO_MACRO_SELECTOR(OATPP_MACRO_DTO_FIELD_, (__VA_ARGS__)) (TYPE, __VA_ARGS__))

// DTO_FIELD_INFO

#define DTO_FIELD_INFO(NAME) \
\
static bool Z__PROPERTY_INIT_##NAME(int, ...) { \
  Z__PROPERTY_INIT_##NAME(); /* call first initialization */ \
  Z__PROPERTY_ADD_INFO_##NAME(&Z__PROPERTY_SINGLETON_##NAME()->info); \
  return true; \
} \
\
static void Z__PROPERTY_ADD_INFO_##NAME(oatpp::data::type::BaseObject::Property::Info* info)


#define DTO_FIELD_TYPE_SELECTOR(NAME) \
\
class Z__PROPERTY_TYPE_SELECTOR_##NAME : public oatpp::BaseObject::Property::FieldTypeSelector<Z__CLASS> { \
public: \
  const oatpp::Type* selectFieldType(Z__CLASS* self) override { \
    return self->Z__PROPERTY_TYPE_SELECTOR_METHOD_##NAME(); \
  } \
}; \
\
static bool Z__PROPERTY_INIT_##NAME(int, int) { \
  Z__PROPERTY_INIT_##NAME(1); /* call property info initialization */ \
  Z__PROPERTY_SINGLETON_##NAME()->info.typeSelector = new Z__PROPERTY_TYPE_SELECTOR_##NAME(); \
  return true; \
} \
\
const oatpp::Type* Z__PROPERTY_TYPE_SELECTOR_METHOD_##NAME()

// FOR EACH

#define OATPP_MACRO_DTO_HC_EQ_PARAM_HC(INDEX, COUNT, X) \
result = ((result << 5) - result) + std::hash<decltype(X)>{}(X);

#define OATPP_MACRO_DTO_HC_EQ_PARAM_EQ(INDEX, COUNT, X) \
&& X == other.X

#define DTO_HASHCODE_AND_EQUALS(...) \
v_uint64 defaultHashCode() const override { \
  return 1; \
} \
\
bool defaultEquals(const DTO&) const override { \
  return true; \
} \
\
v_uint64 hashCode() const { \
  v_uint64 result = 1; \
  result = ((result << 5) - result) + static_cast<const Z__CLASS_EXTENDED&>(*this).hashCode(); \
  OATPP_MACRO_FOREACH(OATPP_MACRO_DTO_HC_EQ_PARAM_HC, __VA_ARGS__) \
  return result; \
} \
\
bool operator==(const Z__CLASS& other) const { \
  return static_cast<const Z__CLASS_EXTENDED&>(*this) == static_cast<const Z__CLASS_EXTENDED&>(other) \
  OATPP_MACRO_FOREACH(OATPP_MACRO_DTO_HC_EQ_PARAM_EQ, __VA_ARGS__) \
  ; \
} \
\
bool operator!=(const Z__CLASS& other) const { \
  return !this->operator==(other); \
}

/**
 * Hashcode and Equals macro. <br>
 * List DTO-fields which should count in hashcode and equals operators.
 */
#define DTO_HC_EQ(...) DTO_HASHCODE_AND_EQUALS(__VA_ARGS__)
