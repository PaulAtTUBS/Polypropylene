//
// Created by Paul on 02.07.2019.
//

#include <sstream>
#include <polypropylene/json/JsonUtil.h>
#include <polypropylene/json/Json.h>

namespace PAX {
    std::string JsonToString(const nlohmann::json & j) {
        if (j.is_string()) {
            return j;
        }

        return j.dump();
    }

    nlohmann::json StringToJson(const std::string & s) {
        bool is_literal = s.find('[') == std::string::npos && s.find('{') == std::string::npos;

        if (is_literal) {
            return s;
        } else {
            return nlohmann::json::parse(s);
        }
    }
}