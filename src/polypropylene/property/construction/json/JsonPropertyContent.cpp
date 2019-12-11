//
// Created by Paul on 02.03.2019.
//

#include <polypropylene/property/construction/json/JsonPropertyContent.h>
#include <polypropylene/json/JsonUtil.h>

namespace PAX {
    namespace Json {
        JsonPropertyContent::JsonPropertyContent(const nlohmann::json &node)
                : Internal::PropertyContent(), node(node) {}

        JsonPropertyContent::~JsonPropertyContent() = default;

        bool JsonPropertyContent::has(const std::string &name) {
            return node.count(name) > 0;
        }

        std::string JsonPropertyContent::getValue(const std::string &key, const VariableRegister & variables) {
            return VariableResolver::resolveVariables(JsonToString(node[key]), variables);
        }

        std::vector<std::string> JsonPropertyContent::getValues(const std::string &key, const VariableRegister & variables) {
            nlohmann::json & keynode = node[key];

            if (keynode.is_array()) {
                std::vector<std::string> values;
                for (auto & it : keynode) {
                    values.push_back(VariableResolver::resolveVariables(JsonToString(it), variables));
                }

                return values;
            }

            return {};
        }

        static void buildVariableHierarchy(VariableHierarchy &h, nlohmann::json &node) {
            for (auto &entry : node.items()) {
                if (entry.value().is_string()) {
                    h.values[entry.key()] = entry.value();
                    //std::cout << "\t" << entry.key() << " -> " << entry.value() << std::endl;
                } else {
                    h.children[entry.key()] = VariableHierarchy();
                    buildVariableHierarchy(h.children[entry.key()], entry.value());
                }
            }
        }

        VariableHierarchy JsonPropertyContent::getResourceParametersFor(const std::string &name) {
            nlohmann::json &child = node[name];
            VariableHierarchy params;

            //std::cout << "[JsonPropertyContent::getResourceParametersFor] " << name << std::endl;

            buildVariableHierarchy(params, child);

            return params;
        }
    }
}