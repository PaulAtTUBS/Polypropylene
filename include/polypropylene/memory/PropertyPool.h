//
// Created by Paul on 13.08.2019.
//

#ifndef POLYPROPYLENE_PROPERTYPOOL_H
#define POLYPROPYLENE_PROPERTYPOOL_H

#include "AllocationService.h"
#include "allocators/PoolAllocator.h"

namespace PAX {
    struct PAX_MAYBEUNUSED DefaultChunkValidator {
        PAX_NODISCARD bool isValid(const PoolAllocator & pool, PoolAllocator::Index i) const;
    };

    /**
     * Iterator for pool allocators.
     * This is a naive implementations that steps over all memory chunks in a pool allocator
     * and inspects if these are allocated or not.
     * Returns allocated chunks and skips invalid ones.
     * @tparam PropertyType The type of property this iterator iterates over.
     */
    template<typename PropertyType, typename ValidatorType = DefaultChunkValidator>
    struct PAX_MAYBEUNUSED PropertyPoolIterator {
    private:
        PoolAllocator & pool;
        PoolAllocator::Index current;
        const ValidatorType & validator;

        PropertyPoolIterator(PoolAllocator & pool, PoolAllocator::Index pos, const ValidatorType & validator)
                : pool(pool), current(pos), validator(validator) {}

    public:
        using Validator = ValidatorType;

        static PropertyPoolIterator BeginOf(PoolAllocator & pool, const ValidatorType & validator) {
            PoolAllocator::Index current = pool.begin();
            while (current < pool.end() && !validator.isValid(pool, current)) {
                ++current;
            }
            return PropertyPoolIterator(pool, current, validator);
        }

        static PropertyPoolIterator EndOf(PoolAllocator & pool, const ValidatorType & validator) {
            return PropertyPoolIterator(pool, pool.end(), validator);
        }

        PropertyPoolIterator & operator=(const PropertyPoolIterator & other) = default;

        inline bool operator==(const PropertyPoolIterator & other) {
            return current == other.current;
        }

        inline bool operator!=(const PropertyPoolIterator & other) {
            return !operator==(other);
        }

        PropertyType * operator*() {
            return reinterpret_cast<PropertyType*>(pool.getData(current));
        }

        PropertyPoolIterator & operator++() {
            // Step over all invalid memory chunks.
            if (current < pool.end()) {
                do {
                    ++current;
                } while (current < pool.end() && !validator.isValid(pool, current));
            }
            return *this;
        }
    };

    /**
     * Convenience class for easy access to all allocated properties of the given type.
     * A PropertyPool registers itself as the default allocator for properties of the given type.
     * Iterating over the pool yields all active properties that are currently in use.
     * Note that only properties allocated via the allocation service of the corresponding entity
     * (@ref Entity<T>::GetAllocationService()) are known to this pool (i.e., in it).
     * @tparam PropertyType The type of properties that should be allocated by this pool.
     * For the specified property type a pool allocator is registered in the allocation service of the corresponding
     * entity.
     */
    template<class PropertyType, class IteratorType = PropertyPoolIterator<PropertyType>>
    class PropertyPool {
        static constexpr size_t PropSize = sizeof(PropertyType);
        std::shared_ptr<PoolAllocator> pool;

    public:
        using Property = PropertyType;
        using Iterator = IteratorType;
        using Validator = typename Iterator::Validator;

        PropertyPool() {
            const Type propType = paxtypeof(Property);
            AllocationService & allocationService = Property::EntityType::GetAllocationService();
            const std::shared_ptr<Allocator> & existingAllocator = allocationService.getAllocator(propType.id);

            // If there is already an allocator registered for our property type ...
            if (existingAllocator) {
                // ... See if it is a PoolAllocator.
                pool = std::dynamic_pointer_cast<PoolAllocator>(existingAllocator);
                if (!pool) {
                    // If it is not, remove it because we will replace it.
                    allocationService.unregisterAllocator(propType.id);
                }
            }

            // If we couldn't reuse an existing allocator.
            if (!pool) {
                // create one
                pool = std::make_shared<PoolAllocator>(propType.name(), PropSize);
                allocationService.registerAllocator(propType.id, pool);
            }
        }

        PAX_NODISCARD virtual const Validator & getValidator() const {
            static Validator v;
            return v;
        }

        Iterator begin() const { return Iterator::BeginOf(*pool, getValidator()); }
        Iterator end() const { return Iterator::EndOf(*pool, getValidator()); }
    };
}

#endif //POLYPROPYLENE_PROPERTYPOOL_H
