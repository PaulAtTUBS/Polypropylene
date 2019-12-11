//
// Created by Paul on 23.09.2017.
//

#ifndef PAXENGINE3_TYPEMAP_H
#define PAXENGINE3_TYPEMAP_H

#include <typeindex>
#include <map>
#include <unordered_map>
#include <proteus/reflection/TypeHandle.h>

namespace PAX {
#define PAX_TYPE_MAP_LIGHTWEIGHT

#ifdef PAX_TYPE_MAP_LIGHTWEIGHT
    template<typename ValueType>
    using TypeMap = std::map<TypeHandle, ValueType>;

    template<typename ValueType>
    using UnorderedTypeMap = std::unordered_map<TypeHandle, ValueType>;
#else
    template<typename ValueType, class Map = std::unordered_map<std::type_index, ValueType>>
    class TypeMap {
        Map _map;

    public:
        using iterator = typename Map::iterator;
        using const_iterator = typename Map::const_iterator;

        template<typename Value>
        inline bool contains() const {
            return _map.find(paxtypeof(Value)) != _map.end();
        }

        inline bool contains(const std::type_index &index) const {
            return _map.find(index) != _map.end();
        }

        template<typename Key>
        inline ValueType& get() {
            return _map.at(paxtypeof(Key));
        }

        ValueType& get(const std::type_index &index) {
            return _map.at(index);
        }

        ValueType& operator[](const std::type_index &index) {
            return _map[index];
        }

        template<typename Key>
        bool put(const ValueType & value) {
            _map[paxtypeof(Key)] = value;
            return true;
        }

        bool put(const std::type_index &index, const ValueType & value) {
            _map[index] = value;
            return true;
        }

        /// Returns the number of erased elements
        template<typename Key>
        size_t erase() {
            return _map.erase(paxtypeof(Key));
        }

        size_t erase(const std::type_index &index) {
            return _map.erase(index);
        }

        bool empty() const {
            return _map.empty();
        }

        void clear() {
            _map.clear();
        }

        iterator begin() { return _map.begin(); }
        iterator end() { return _map.end(); }
        const_iterator begin() const { return _map.begin(); }
        const_iterator end() const { return _map.end(); }
        const_iterator cbegin() const { return _map.cbegin(); }
        const_iterator cend() const { return _map.cend(); }
    };
#endif
}

#endif //PAXENGINE3_TYPEMAP_H
