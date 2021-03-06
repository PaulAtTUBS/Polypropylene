//
// Created by paul on 06.01.19.
//

#ifndef POLYPROPYLENE_PROPERTYFACTORY_H
#define POLYPROPYLENE_PROPERTYFACTORY_H

#include <string>
#include <unordered_map>

#include "polypropylene/property/Creation.h"
#include "polypropylene/log/Errors.h"
#include "polypropylene/reflection/TypeMap.h"
#include "polypropylene/serialisation/ClassMetadataSerialiser.h"

// Instead of 'if constexpr' we could use PAX_CONSTEXPR_IF here
// but the following won't compile without 'constexpr'.
#define PAX_PROPERTY_REGISTER_AS(TPropertyType, Name) \
do {                                                  \
    if constexpr (!TPropertyType::IsAbstract()) {     \
        ::PAX::PropertyFactoryRegister<TPropertyType::EntityType>::registerFactory<TPropertyType>(Name); \
    } \
} while(0)

/**
 * Registers the given property type at the property factory.
 * This has to be done for each property that should be used within
 * prefabs and serialization.
 * This can be done once on program start (e.g., at the beginning of the main function).
 */
#define PAX_PROPERTY_REGISTER(TPropertyType) PAX_PROPERTY_REGISTER_AS(TPropertyType, #TPropertyType)

namespace PAX {
    template<class TEntityType>
    class IPropertyFactory {
        std::string name;

    protected:
        explicit IPropertyFactory(const std::string & name) noexcept : name(name) {}
        virtual ~IPropertyFactory() = default;

    public:
        /**
         * Creates a property allocated with the allocator of Entity<TEntityType>.
         * @return A newly heap-allocated Property.
         */
        PAX_NODISCARD virtual typename TEntityType::PropertyType * create() const = 0;
        PAX_NODISCARD virtual Type getPropertyType() const = 0;
        PAX_NODISCARD virtual bool isPropertyMultiple() const = 0;

        PAX_NODISCARD const std::string & getPropertyName() const {
            return name;
        }
    };

    template<class TEntityType>
    class PropertyFactoryRegister {
        // Use this method to save the map to avoid the Static Initialization Order Fiasko.
        static std::unordered_map<std::string, IPropertyFactory<TEntityType> *> & getNameMap() noexcept {
            static std::unordered_map<std::string, IPropertyFactory<TEntityType> *> map;
            return map;
        }

        // Use this method to save the map to avoid the Static Initialization Order Fiasko.
        static UnorderedTypeMap<IPropertyFactory<TEntityType> *> & getTypeMap() noexcept {
            static UnorderedTypeMap<IPropertyFactory<TEntityType> *> map;
            return map;
        }

    protected:
        PropertyFactoryRegister() noexcept = default;

    public:
        virtual ~PropertyFactoryRegister() = default;

        PAX_NODISCARD static IPropertyFactory<TEntityType> * getFactoryFor(const std::string & name) {
            const auto &map = getNameMap();
            const auto &it = map.find(name);

            if (it != map.end()) {
                return it->second;
            } else {
                PAX_THROW_RUNTIME_ERROR("No factory is registered for the name \"" << name << "\"!");
            }
        }

        PAX_NODISCARD static IPropertyFactory<TEntityType> * getFactoryFor(const TypeId & type) {
            const auto &map = getTypeMap();
            const auto &it = map.find(type);

            if (it != map.end()) {
                return it->second;
            } else {
                PAX_THROW_RUNTIME_ERROR("No factory is registered for the type \"" << type.name() << "\"!");
            }
        }

        /**
         * Returns a map containing all registered IPropertyFactories, identified by their name.
         * The factories in the map are the same as for getFactoriesByType() but the key is different.
         * @return All registered IPropertyFactories indexed by the name of the type of property they produce.
         */
        PAX_NODISCARD static const std::unordered_map<std::string, IPropertyFactory<TEntityType> *> & getFactoriesByName() {
            return getNameMap();
        }

        /**
         * Returns a map containing all registered IPropertyFactories, identified by their type.
         * The factories in the map are the same as for getFactoriesByName() but the key is different.
         * @return All registered IPropertyFactories indexed by the type of property they produce.
         */
        PAX_NODISCARD static const UnorderedTypeMap<IPropertyFactory<TEntityType> *> & getFactoriesByType() {
            return getTypeMap();
        }

        template<typename TPropertyType>
        static void registerFactory(const std::string & name);
    };

    template<typename TPropertyType, typename TEntityType>
    class PropertyFactory : public IPropertyFactory<TEntityType> {
    public:
        explicit PropertyFactory(const std::string & name) noexcept : IPropertyFactory<TEntityType>(name) {}
        virtual ~PropertyFactory() = default;

        PAX_NODISCARD TPropertyType * create() const override {
            return pax_new(TPropertyType)();
        }

        PAX_NODISCARD Type getPropertyType() const override {
            return paxtypeof(TPropertyType);
        }

        PAX_NODISCARD bool isPropertyMultiple() const override {
            return TPropertyType::IsMultiple();
        }
    };

    template<class TEntityType>
    template<typename TPropertyType>
    void PropertyFactoryRegister<TEntityType>::registerFactory(const std::string & name) {
        static PropertyFactory<TPropertyType, TEntityType> factory(name);
        PropertyFactoryRegister<TEntityType>::getNameMap()[name] = &factory;
        PropertyFactoryRegister<TEntityType>::getTypeMap()[typeid(TPropertyType)] = &factory;
    }
}

#endif