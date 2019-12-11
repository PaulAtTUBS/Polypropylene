//
// Created by Bittner on 21/05/2019.
//

#ifndef PAXENGINE3_JSONPARSER_H
#define PAXENGINE3_JSONPARSER_H

#include "JsonUtil.h"
#include "../TryParser.h"

namespace PAX {
    template<class To>
    class TryParser<nlohmann::json, To> {
    public:
        PAX_NODISCARD static To tryParse(const nlohmann::json & f) {
            return String::tryParse<To>(JsonToString(f));
        }
    };

    namespace Json {
        template<typename T>
        PAX_NODISCARD T tryParse(const nlohmann::json & j) {
            return TryParser<nlohmann::json, T>::tryParse(j);
        }
    }

#define PAX_SPECIALIZE_JSONTRYPARSE_HEADER(type) \
            template<> \
            class TryParser<nlohmann::json, type> { \
            public: \
                PAX_NODISCARD static type tryParse(const nlohmann::json & j); \
            };
}

#endif //PAXENGINE3_JSONPARSER_H
