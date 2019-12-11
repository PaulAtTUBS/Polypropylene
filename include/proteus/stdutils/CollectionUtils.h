//
// Created by Paul on 28.05.2017.
//

#ifndef POLYPROPYLENE_STDUTILS_H
#define POLYPROPYLENE_STDUTILS_H

#include <vector>
#include <algorithm>
#include <unordered_map>

namespace PAX {
    namespace Util {

        template<typename Key, typename Value>
        inline bool removeFromMap(std::unordered_map<Key, Value> &map, Key &key) {
            auto iterator = map.find(key);
            if (iterator != map.end()) {
                map.erase(iterator);
                return true;
            }
            return false;
        };

        template<class T>
        inline bool removeFromVector(std::vector<T> &vector, const T &element) {
            typename std::vector<T>::iterator iter = std::find(vector.begin(), vector.end(), element);
            if (iter != vector.end()) {
                vector.erase(iter);
                return true;
            }
            return false;
        }

        template<class T>
        inline bool vectorContains(std::vector<T> &vector, const T &element) {
            return std::find(vector.begin(), vector.end(), element) != vector.end();
        }
    }
}

#endif //POLYPROPYLENE_STDUTILS_H
