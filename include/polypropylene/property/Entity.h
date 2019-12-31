//
// Created by paul on 22.09.18.
//

#ifndef POLYPROPYLENE_ENTITY_H
#define POLYPROPYLENE_ENTITY_H

#include <cassert>
#include <vector>
#include <optional>

#include "../definitions/CompilerDetection.h"
#include "../memory/AllocationService.h"
#include "../reflection/TypeMap.h"
#include "../event/EventService.h"

#include "PrototypeEntityPrefab.h"

// We have to create this workaround, because MSVC can't handle constexpr functions in enable_if.
#ifdef PAX_COMPILER_MSVC
#define PAX_GENERATE_EntityTemplateHeader(rettype, neg) \
template <class PropertyType, bool mult = PropertyType::IsMultiple()> \
typename std::enable_if<neg mult, rettype>::type
#else
#define PAX_GENERATE_EntityTemplateHeader(rettype, neg) \
template <class PropertyType> \
typename std::enable_if<neg PropertyType::IsMultiple(), rettype>::type
#endif

namespace PAX {
    /**
     * An Entity is a generic container for Properties.
     * @tparam Derived The class deriving from Entity.
     */
    template<class Derived>
    class Entity {
        static const std::vector<Property<Derived>*> EmptyPropertyVector;
        static AllocationService propertyAllocator;

        EventService localEventService;

        TypeMap<Property<Derived>*> singleProperties;
        TypeMap<std::vector<Property<Derived>*>> multipleProperties;

        std::vector<Property<Derived>*> allProperties; // Do we really want to have a copy of all pointers for easy access?

    public:
        Entity() = default;

        /**
         * Deletes all properties.
         */
        virtual ~Entity() {
            std::vector<Property<Derived>*> myProperties = getAllProperties();
            AllocationService & allocator = GetPropertyAllocator();

            while (!myProperties.empty()) {
                Property<Derived> * victim = myProperties.back();
                myProperties.pop_back();

                if (allocator.hasAllocated(victim)) {
                    TypeHandle victimType = victim->getClassType();
                    victim->~Property<Derived>();
                    GetPropertyAllocator().free(victimType.id, victim);
                }
            }
        }

    private:
        bool isValid(Property<Derived>* property) {
            /*
            if (property->owner)
                return false;
            //*/

            return property->areDependenciesMetFor(*static_cast<Derived*>(this));
        }

    public:
        /**
         * @return the AllocationService that is used for property (de-) allocation for properties for derived entitiy type Derived.
         */
        PAX_NODISCARD static AllocationService& GetPropertyAllocator() {
            return propertyAllocator;
        }

        /**
         * @return The internal EventService of this Entity that is used for internal communication between properties.
         */
        PAX_NODISCARD EventService& getEventService() {
            return localEventService;
        }

        PAX_NODISCARD PrototypeEntityPrefab<Derived> toPrefab() const {
            return PrototypeEntityPrefab<Derived>(*this);
        }
        
        bool add(Property<Derived>* property) {
            if (isValid(property)) {
                property->PAX_INTERNAL(addTo)(*static_cast<Derived*>(this));
                property->attached(*static_cast<Derived*>(this));
                allProperties.push_back(property);
                return true;
            }

            return false;
        }

        bool remove(Property<Derived>* property) {
            bool ret = property->PAX_INTERNAL(removeFrom)(*static_cast<Derived*>(this));
            property->detached(*static_cast<Derived*>(this));
            Util::removeFromVector(allProperties, property);
            return ret;
        }

        PAX_GENERATE_EntityTemplateHeader(bool, !)
        has() const {
            return singleProperties.count(typeid(PropertyType)) > 0;
        }

        PAX_GENERATE_EntityTemplateHeader(bool, )
        has() const {
            return multipleProperties.count(typeid(PropertyType)) > 0;
        }

        template<class FirstPropertyType, class SecondPropertyType, class... FurtherPropertyTypees>
        PAX_NODISCARD bool has() const {
            bool X[] = { has<FirstPropertyType>(), has<SecondPropertyType>(), has<FurtherPropertyTypees>()... };

            constexpr int len = sizeof...(FurtherPropertyTypees) + 2;
            for (int i = 0; i < len; ++i)
                if (!X[i]) return false;

            return true;
        }

        PAX_NODISCARD bool has(const TypeId & type, std::optional<bool> isMultipleHint = {}) const {
            bool ret = false;

            if (isMultipleHint.value_or(true)) {
                ret = multipleProperties.count(type) > 0;
                if (ret || isMultipleHint.has_value())
                    return ret;
            }

            assert(!ret);

            if (!isMultipleHint.value_or(false)) {
                ret = singleProperties.count(type) > 0;
            }

            return ret;
        }

        PAX_GENERATE_EntityTemplateHeader(PropertyType*, !)
        get() {
            const auto& property = singleProperties.find(typeid(PropertyType));
            if (property != singleProperties.end())
                return static_cast<PropertyType*>(property->second);
            return nullptr;
        }

        PAX_GENERATE_EntityTemplateHeader(const std::vector<PropertyType*>&, )
        get() {
            const auto& properties = multipleProperties.find(typeid(PropertyType));
            if (properties != multipleProperties.end())
                return reinterpret_cast<std::vector<PropertyType*>&>(properties->second);
            else
                return *reinterpret_cast<const std::vector<PropertyType*>*>(&EmptyPropertyVector);
        }

        PAX_NODISCARD const std::vector<Property<Derived>*> & getAllProperties() const {
            return allProperties;
        }

        /**
         * Returns a property from its runtime typehandle.
         * @param type The runtime type of the requested property.
         * @return The property of the given type which has single multiplicity (PAX_PROPERTY_IS_SINGLE), null if
         * this entity does not contain a property of that type or the type is not single.
         */
        PAX_NODISCARD Property<Derived> * getSingle(const TypeId & type) const {
            const auto & it = singleProperties.find(type);

            if (it != singleProperties.end()) {
                return it->second;
            }

            return nullptr;
        }

        /**
         * Returns a property from its runtime typehandle.
         * @param type The runtime type of the requested property.
         * @return The properties of the given type which has multiple multiplicity (PAX_PROPERTY_IS_MULTIPLE), an
         * empty vector if this entity does not contain a property of that type or the type is not multiple.
         */
        PAX_NODISCARD const std::vector<Property<Derived>*> & getMultiple(const TypeId & type) const {
            const auto & it = multipleProperties.find(type);

            if (it != multipleProperties.end()) {
                return it->second;
            }

            return EmptyPropertyVector;
        }

        /**
         * Returns all properties of the given type contained in this entity.
         * @param type The runtime type of the requested properties.
         * @return The properties of the given type with any multiplicity.
         * The returned vector is empty, iff this entity does not contain any properties of the given type.
         * If the property type is single (PAX_PROPERTY_IS_SINGLE), the returned vector will have size 1.
         * If the property type is multiple (PAX_PROPERTY_IS_MULTIPLE), the returned vector will contain all properties
         * of the given type.
         */
        std::vector<Property<Derived>*> get(const TypeId & type) {
            // Copy is intended
            std::vector<Property<Derived>*> props = getMultiple(type);
            if (Property<Derived> * single = getSingle(type)) {
                props.emplace_back(single);
            }
            return props;
        }

        PAX_GENERATE_EntityTemplateHeader(PropertyType*, !)
        removeAll() {
            const auto& propertyIt = singleProperties.contains(typeid(PropertyType));
            if (propertyIt != singleProperties.end()) {
                PropertyType* property = static_cast<PropertyType*>(propertyIt->second);
                if (remove(property))
                    return property;
            }

            return nullptr;
        }

        PAX_GENERATE_EntityTemplateHeader(const std::vector<PropertyType*>&, )
        removeAll() {
            const auto& propertiesIt = multipleProperties.contains(typeid(PropertyType));
            if (propertiesIt != multipleProperties.end()) {
                // Copy to be able to return all removed instances
                auto properties = reinterpret_cast<std::vector<PropertyType*>>(multipleProperties.get(typeid(PropertyType)));
                for (PropertyType* property : properties) {
                    if (!remove(property))
                        return EmptyPropertyVector;
                }

                return properties;
            }

            return EmptyPropertyVector;
        }

        
        /// DANGER ZONE: Functions for internal use only !!!!!!!!!!!!!!

        bool PAX_INTERNAL(addAsMultiple)(const TypeId & type, Property<Derived>* property) {
            multipleProperties[type].push_back(property);
            return true;
        }

        bool PAX_INTERNAL(addAsSingle)(const TypeId & type, Property<Derived>* property) {
            if (singleProperties.count(type)) {
                return false;
            } else {
                singleProperties[type] = property;
            }

            return true;
        }

        bool PAX_INTERNAL(removeAsMultiple)(const TypeId & type, Property<Derived>* property) {
            std::vector<Property<Derived>*> &result = multipleProperties.at(type);
            if (!Util::removeFromVector(result, property)) {
                return false;
            }

            // Remove vector if no propertys remain
            if (result.empty()) {
                multipleProperties.erase(type);
            }

            return true;
        }

        bool PAX_INTERNAL(removeAsSingle)(const TypeId & type, Property<Derived>* property) {
            // The given property is not the property, that is registered for the given type.
            if (singleProperties.at(type) != property) {
                return false;
            } else {
                return singleProperties.erase(type) != 0;
            }
        }
    };

    template <class C>
    AllocationService Entity<C>::propertyAllocator;

    template <class C>
    const std::vector<Property<C>*> Entity<C>::EmptyPropertyVector(0);
}

#undef PAX_GENERATE_PropertyContainerFunctionTemplateHeader

#endif //POLYPROPYLENE_ENTITY_H
