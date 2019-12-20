//
// Created by Bittner on 20.12.2019.
//

#include <polypropylene/json/JsonParser.h>
#include <polypropylene/log/Errors.h>

namespace PAX {
    JsonParserRegister * JsonParserRegister::instance = nullptr;

    JsonParserRegister * JsonParserRegister::Instance() {
        if (!instance) {
            instance = new JsonParserRegister();
        }

        return instance;
    }

    IJsonParser * JsonParserRegister::getParserFor(const PAX::TypeHandle & type) const {
        const auto & it = parsers.find(type);
        if (it != parsers.end()) {
            return it->second;
        }

        PAX_THROW_RUNTIME_ERROR("No IJsonParser registered for type: " << type.name());
    }
}