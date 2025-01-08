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

#ifndef oatpp_data_type_Object_hpp
#define oatpp_data_type_Object_hpp

#include "./Type.hpp"

#include "./Tree.hpp"
#include "./Any.hpp"
#include "./Primitive.hpp"
#include "./Enum.hpp"
#include "./UnorderedMap.hpp"
#include "./PairList.hpp"
#include "./List.hpp"
#include "./Vector.hpp"
#include "./UnorderedSet.hpp"

#include "oatpp/base/Countable.hpp"

#include <type_traits>

namespace oatpp { namespace data { namespace type {

/**
 * Base class of all object-like mapping-enabled structures ex.: oatpp::DTO.
 */
class BaseObject : public oatpp::base::Countable {
public:

  /**
   * Class to map object properties.
   */
  class Property {
  public:

    /**
     * Property Type Selector.
     */
    class TypeSelector {
    public:

      /**
       * Default destructor.
       */
      virtual ~TypeSelector() = default;

      /**
       * Select property type.
       * @param self - pointer to `this` object.
       * @return - &id:oatpp::Type;.
       */
      virtual const type::Type* selectType(BaseObject* self) = 0;
    };

    template<class DTOType>
    class FieldTypeSelector : public TypeSelector {
    public:

      const type::Type* selectType(BaseObject* self) override {
        return selectFieldType(static_cast<DTOType*>(self));
      }

      virtual const type::Type* selectFieldType(DTOType* self) = 0;

    };

  public:

    /**
     * Editional Info about Property.
     */
    struct Info {

      /**
       * Description.
       */
      std::string description = "";

      /**
       * Pattern.
       */
      std::string pattern = "";

      /**
       * Required.
       */
      bool required = false;

      /**
       * Type selector.
       * &l:Property::TypeSelector;.
       */
      TypeSelector* typeSelector = nullptr;

    };

  private:
    const v_int64 offset;
  public:

    /**
     * Constructor.
     * @param pOffset - memory offset of object field from object start address.
     * @param pName - name of the property.
     * @param pUName - unqualified name of the property. Name of the class field in the code.
     * @param pType - &l:Type; of the property.
     */
    Property(v_int64 pOffset, std::string pName, std::string pUName, const Type* pType);

    /**
     * Property name.
     */
    const std::string name;

    /**
     * Unqualified property name.
     * Name of the class field that is used in the code.
     */
    const std::string unqualifiedName;

    /**
     * Property type.
     */
    const Type* const type;

    /**
     * Property additional info.
     */
    Info info;

    /**
     * Set value of object field mapped by this property.
     * @param object - object address.
     * @param value - value to set.
     */
    void set(BaseObject* object, const Void& value) const;

    /**
     * Get value of object field mapped by this property.
     * @param object - object address.
     * @return - value of the field.
     */
    Void get(BaseObject* object) const;

    /**
     * Get reference to ObjectWrapper of the object field.
     * @param object - object address.
     * @return - reference to ObjectWrapper of the object field.
     */
    Void& getAsRef(BaseObject* object) const;

  };

  /**
   * Object type properties table.
   */
  class Properties {
  private:
    std::unordered_map<std::string, Property*> m_map;
    std::unordered_map<std::string, Property*> m_unqualifiedMap;
    std::list<Property*> m_list;
  public:

    /**
     * Add property to the end of the list.
     * @param property
     */
    Property* pushBack(Property* property);

    /**
     * Add all properties to the beginning of the list.
     * @param properties
     */
    void pushFrontAll(Properties* properties);

    /**
     * Get properties as unordered map for random access.
     * @return reference to std::unordered_map of std::string to &id:oatpp::data::type::BaseObject::Property;*.
     */
    const std::unordered_map<std::string, Property*>& getMap() const;

    /**
     * Get properties as unordered map for random access by unqualified names.
     * @return reference to std::unordered_map of std::string to &id:oatpp::data::type::BaseObject::Property;*.
     */
    const std::unordered_map<std::string, Property*>& getUnqualifiedMap() const;

    /**
     * Get properties in ordered way.
     * @return std::list of &id:oatpp::data::type::BaseObject::Property;*.
     */
    const std::list<Property*>& getList() const;

  };

private:
  void* m_basePointer = this;
private:
  void set(v_int64 offset, const Void& value);
  Void get(v_int64 offset) const;
  Void& getAsRef(v_int64 offset) const;
protected:
  void setBasePointer(void* basePointer);
  void* getBasePointer() const;
};

namespace __class {

  /**
   * AbstractObject class.
   */
  class AbstractObject {
  public:

    class PolymorphicDispatcher {
    public:
      
      virtual ~PolymorphicDispatcher() = default;

      virtual type::Void createObject() const = 0;

      virtual const type::BaseObject::Properties* getProperties() const = 0;

    };

  public:

    /**
     * Class id.
     */
    static const ClassId CLASS_ID;

  };

  /**
   * Template for Object class of type T.
   * @tparam T - object type.
   */
  template<class T>
  class Object : public AbstractObject {
  public:

    class PolymorphicDispatcher : public AbstractObject::PolymorphicDispatcher {
    public:

      type::Void createObject() const override {
        return type::Void(std::make_shared<T>(), getType());
      }

      const type::BaseObject::Properties* getProperties() const override {
        return propertiesGetter();
      }

    };

  private:

    static type::BaseObject::Properties* initProperties() {

      /* initializer */
      T obj;

      /* init parent properties */
      auto parentType = Object<typename T::Z__CLASS_EXTENDED>::getType();
      if(parentType->parent != nullptr) {
        auto dispatcher = static_cast<const AbstractObject::PolymorphicDispatcher*>(parentType->polymorphicDispatcher);
        dispatcher->getProperties();
      }

      /* extend parent properties */
      T::Z__CLASS_EXTEND(T::Z__CLASS::Z__CLASS_GET_FIELDS_MAP(), T::Z__CLASS_EXTENDED::Z__CLASS_GET_FIELDS_MAP());

      return T::Z__CLASS::Z__CLASS_GET_FIELDS_MAP();

    }

    static const BaseObject::Properties* propertiesGetter() {
      static type::BaseObject::Properties* properties = initProperties();
      return properties;
    }

    static std::unique_ptr<Type> createType() {
      Type::Info info;
      info.nameQualifier = T::Z__CLASS_TYPE_NAME();
      static std::unique_ptr<PolymorphicDispatcher> disp(new PolymorphicDispatcher());
      info.polymorphicDispatcher = disp.get();
      info.parent = T::getParentType();
      return std::unique_ptr<Type>(new Type(CLASS_ID, info));
    }

  public:

    /**
     * Get type describing this class.
     * @return - &id:oatpp::data::type::Type;
     */
    static Type* getType() {
      static std::unique_ptr<Type> type = createType();
      return type.get();
    }
    
  };
  
}

/**
 * ObjectWrapper for &l:DTO;. AKA `oatpp::Object<T>`.
 * @tparam ObjT - class extended from &l:DTO;.
 */
template<class ObjT>
class DTOWrapper : public ObjectWrapper<ObjT, __class::Object<ObjT>> {
  template<class Type>
  friend class DTOWrapper;
public:
  typedef ObjT TemplateObjectType;
  typedef __class::Object<ObjT> TemplateObjectClass;
public:

  OATPP_DEFINE_OBJECT_WRAPPER_DEFAULTS(DTOWrapper, TemplateObjectType, TemplateObjectClass)

  template<class OtherT>
  DTOWrapper(const OtherT& other)
    : type::ObjectWrapper<ObjT, __class::Object<ObjT>>(other.m_ptr)
  {}

  template<class OtherT>
  DTOWrapper(OtherT&& other)
    : type::ObjectWrapper<ObjT, __class::Object<ObjT>>(std::move(other.m_ptr))
  {}

  static DTOWrapper createShared() {
    return std::make_shared<TemplateObjectType>();
  }

  template<class T>
  DTOWrapper& operator = (const DTOWrapper<T>& other) {
    this->m_ptr = other.m_ptr;
    return *this;
  }

  template<class T>
  DTOWrapper& operator = (DTOWrapper<T>&& other) {
    this->m_ptr = std::move(other.m_ptr);
    return *this;
  }

  template<typename T,
    typename enabled = typename std::enable_if<std::is_same<T, std::nullptr_t>::value, void>::type
  >
  inline bool operator == (T){
    return this->m_ptr.get() == nullptr;
  }

  template<typename T,
    typename enabled = typename std::enable_if<std::is_same<T, std::nullptr_t>::value, void>::type
  >
  inline bool operator != (T){
    return this->m_ptr.get() != nullptr;
  }

  template<typename T,
    typename enabled = typename std::enable_if<std::is_same<T, DTOWrapper>::value, void>::type
  >
  inline bool operator == (const T &other) const {
    if(this->m_ptr.get() == other.m_ptr.get()) return true;
    if(!this->m_ptr || !other.m_ptr) return false;
    return *this->m_ptr == *other.m_ptr;
  }

  template<typename T,
    typename enabled = typename std::enable_if<std::is_same<T, DTOWrapper>::value, void>::type
  >
  inline bool operator != (const T &other) const {
    return !operator == (other);
  }

  static const std::unordered_map<std::string, BaseObject::Property*>& getPropertiesMap() {
    auto dispatcher = static_cast<const __class::AbstractObject::PolymorphicDispatcher*>(
      __class::Object<ObjT>::getType()->polymorphicDispatcher
    );
    return dispatcher->getProperties()->getMap();
  }

  static const std::list<BaseObject::Property*>& getPropertiesList() {
    auto dispatcher = static_cast<const __class::AbstractObject::PolymorphicDispatcher*>(
      __class::Object<ObjT>::getType()->polymorphicDispatcher
    );
    return dispatcher->getProperties()->getList();
  }

  static v_int64 getPropertiesCount() {
    auto dispatcher = static_cast<const __class::AbstractObject::PolymorphicDispatcher*>(
      __class::Object<ObjT>::getType()->polymorphicDispatcher
    );
    return static_cast<v_int64>(dispatcher->getProperties()->getList().size());
  }

  ObjectWrapper<void>& operator[](const std::string& propertyName) {
    return getPropertiesMap().at(propertyName)->getAsRef(this->m_ptr.get());
  }

};

/**
 * Base class for all DTO objects.
 * For more info about Data Transfer Object (DTO) see [Data Transfer Object (DTO)](https://oatpp.io/docs/components/dto/).
 */
class DTO : public BaseObject {
  template<class T>
  friend class __class::Object;
private:
  typedef DTO Z__CLASS;
  typedef DTO Z__CLASS_EXTENDED;
public:
  typedef oatpp::data::type::Void Void;
  typedef oatpp::data::type::Any Any;
  typedef oatpp::data::type::Tree Tree;
  typedef oatpp::data::type::String String;
  typedef oatpp::data::type::Int8 Int8;
  typedef oatpp::data::type::UInt8 UInt8;
  typedef oatpp::data::type::Int16 Int16;
  typedef oatpp::data::type::UInt16 UInt16;
  typedef oatpp::data::type::Int32 Int32;
  typedef oatpp::data::type::UInt32 UInt32;
  typedef oatpp::data::type::Int64 Int64;
  typedef oatpp::data::type::UInt64 UInt64;
  typedef oatpp::data::type::Float32 Float32;
  typedef oatpp::data::type::Float64 Float64;
  typedef oatpp::data::type::Boolean Boolean;

  template <class T>
  using Object = DTOWrapper<T>;

  template <class T>
  using Enum = oatpp::data::type::Enum<T>;

  template <class T>
  using Vector = oatpp::data::type::Vector<T>;

  template <class T>
  using UnorderedSet = oatpp::data::type::UnorderedSet<T>;

  template <class T>
  using List = oatpp::data::type::List<T>;

  template <class Value>
  using Fields = oatpp::data::type::PairList<String, Value>;

  template <class Value>
  using UnorderedFields = oatpp::data::type::UnorderedMap<String, Value>;

private:
  static const type::Type* getParentType();
  static const char* Z__CLASS_TYPE_NAME();
  static data::type::BaseObject::Properties* Z__CLASS_GET_FIELDS_MAP();
  static BaseObject::Properties* Z__CLASS_EXTEND(BaseObject::Properties* properties, BaseObject::Properties* extensionProperties);
public:

  virtual v_uint64 defaultHashCode() const {
    return reinterpret_cast<v_uint64>(this);
  }

  virtual bool defaultEquals(const DTO& other) const {
    return this == &other;
  }

  v_uint64 hashCode() const {
    return defaultHashCode();
  }

  bool operator==(const DTO& other) const {
    return defaultEquals(other);
  }

};
  
}}}

namespace std {

  template<class T>
  struct hash<oatpp::data::type::DTOWrapper<T>> {

    typedef oatpp::data::type::DTOWrapper<T> argument_type;
    typedef v_uint64 result_type;

    result_type operator()(argument_type const &ow) const noexcept {
      if(ow) {
        return ow->hashCode();
      }
      return 0;
    }

  };

}

#endif /* oatpp_data_type_Object_hpp */
