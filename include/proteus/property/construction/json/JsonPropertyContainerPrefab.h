//
// Created by Bittner on 01/03/2019.
//

#ifndef PAXENGINE3_JSONPROPERTYCONTAINERPREFAB_H
#define PAXENGINE3_JSONPROPERTYCONTAINERPREFAB_H

#include <proteus/property/construction/PropertyContainerPrefab.h>
#include <proteus/json/JsonUtil.h>
#include <proteus/io/Path.h>

#include <proteus/property/Property.h>

#include "../ContentProvider.h"
#include "JsonPropertyContent.h"

namespace PAX {
    namespace Json {
        template<typename C>
        class JsonPropertyContainerPrefab;

        template<typename C>
        using JsonPropertyContainerPrefabElementParser = JsonElementParser<C &, JsonPropertyContainerPrefab<C> &, const VariableRegister &>;

        template<typename C>
        class LambdaJsonPropertyContainerPrefabElementParser : public JsonPropertyContainerPrefabElementParser<C> {
        public:
            using Callback = std::function<void(nlohmann::json &, C &, JsonPropertyContainerPrefab<C> &, const VariableRegister &)>;

        private:
            Callback callback;

        public:
            explicit LambdaJsonPropertyContainerPrefabElementParser(const Callback & function)
                    : JsonPropertyContainerPrefabElementParser<C>(), callback(function) {}

            ~LambdaJsonPropertyContainerPrefabElementParser() override = default;

            void parse(nlohmann::json &j, C &c, JsonPropertyContainerPrefab<C> &prefab, const VariableRegister & variableRegister) override {
                callback(j, c, prefab, variableRegister);
            }
        };

        template<typename C>
        class JsonProperyContainerPrefabElementParserRegister {
            std::map<std::string, JsonPropertyContainerPrefabElementParser<C> *> parsers;

        public:
            JsonProperyContainerPrefabElementParserRegister() = default;

            bool registerParser(const std::string &name, JsonPropertyContainerPrefabElementParser<C> *parser) {
                const auto &it = parsers.find(name);
                if (it == parsers.end()) {
                    parsers[name] = parser;
                    return true;
                }

                return false;
            }

            bool registerParser(const std::string &name,
                                const typename LambdaJsonPropertyContainerPrefabElementParser<C>::Callback &lambda) {
                return registerParser(name, new LambdaJsonPropertyContainerPrefabElementParser<C>(lambda));
            }

            bool unregisterParser(const std::string &name) {
                const auto &it = parsers.find(name);
                if (it != parsers.end()) {
                    parsers.erase(it);
                    return true;
                }

                return false;
            }

            const std::map<std::string, JsonPropertyContainerPrefabElementParser<C> *> & getRegister() {
                return parsers;
            }
        };

        template<typename C>
        class JsonPropertyContainerPrefab : public PropertyContainerPrefab<C> {
            using json = nlohmann::json;

            std::shared_ptr<json> rootNode;
            Path path;

            void parse(json &parent, const std::string &childname, C &c, const VariableRegister & variableRegister) {
                const auto &parserRegister = Parsers.getRegister();
                const auto &it = parserRegister.find(childname);
                if (it != parserRegister.end()) {
                    if (parent.count(childname) > 0) {
                        it->second->parse(parent[childname], c, *this, variableRegister);
                    }
                } else {
                    PAX_LOG(Log::Level::Warn, "ignoring element " << childname << " because no parser is registered for it!");
                }
            }

        public:
            static JsonProperyContainerPrefabElementParserRegister<C> Parsers;

            explicit JsonPropertyContainerPrefab(const std::shared_ptr<json> &file, const Path &path)
                    : PropertyContainerPrefab<C>(), rootNode(file), path(path) {}

            virtual ~JsonPropertyContainerPrefab() = default;

            Path resolvePath(const std::string & str) {
                Path p = Path(VariableResolver::resolveVariables(str, Prefab::PreDefinedVariables));

                if (p.isRelative()) {
                    p = getPath().getDirectory() + p;
                }

                return p;
            }

            static void initialize(Resources &resources) {
                Parsers.registerParser(
                        "Inherits",
                        [&resources](json &node, C &c, JsonPropertyContainerPrefab<C> &prefab, const VariableRegister & variableRegister) {
                            for (auto &el : node.items()) {
                                Path parentPath = prefab.path.getDirectory() + el.value();
                                std::shared_ptr<PropertyContainerPrefab<C>> parentPrefab;

                                const auto &it = prefab.parentPrefabs.find(parentPath);
                                if (it != prefab.parentPrefabs.end()) {
                                    parentPrefab = it->second;
                                } else {
                                    parentPrefab = resources.loadOrGet<PropertyContainerPrefab<C>>(
                                            parentPath);
                                    prefab.parentPrefabs[parentPath] = parentPrefab;
                                }

                                parentPrefab->addMyContentTo(c, variableRegister);
                            }
                        });

                Parsers.registerParser(
                        "Properties",
                        [&resources](json &node, C &c, JsonPropertyContainerPrefab<C> &prefab, const VariableRegister & variableRegister) {
                            std::vector<Property<C> *> props;

                            ContentProvider contentProvider(resources,
                                                            variableRegister);

                            for (auto &el : node.items()) {
                                const std::string propTypeName = el.key();
                                IPropertyFactory<C> *propertyFactory = PropertyFactoryRegister<C>::getFactoryFor(
                                        propTypeName);

                                if (propertyFactory) {
                                    JsonPropertyContent content(el.value());
                                    contentProvider.setContent(&content);

                                    // If the container already has properties of the given type we wont create a new one,
                                    // but instead overwrite the old ones with the newer settings.
                                    const PAX::TypeHandle &propType = propertyFactory->getPropertyType();
                                    bool isPropMultiple = propertyFactory->isPropertyMultiple();

                                    if (c.has(propType, isPropMultiple)) {
                                        // Get the corresponding property/ies
                                        if (isPropMultiple) {
                                            const std::vector<Property<C> *> &existingProperties = c.getMultiple(
                                                    propType);
                                            for (Property<C> *existingProperty : existingProperties) {
                                                propertyFactory->reinit(existingProperty, contentProvider);
                                            }
                                        } else {
                                            propertyFactory->reinit(c.getSingle(propType), contentProvider);
                                        }
                                    } else {
                                        props.emplace_back(propertyFactory->create(contentProvider));
                                    }

                                    contentProvider.setContent(nullptr);
                                }
                            }

                            // Add the properties deferred to resolve their dependencies.
                            while (!props.empty()) {
                                size_t numOfPropsToAdd = props.size();

                                for (auto it = props.begin(); it != props.end(); ++it) {
                                    if ((*it)->areDependenciesMetFor(c)) {
                                        c.add(*it);
                                        props.erase(it);
                                        break;
                                    }
                                }

                                if (numOfPropsToAdd == props.size()) {
                                    // Not a single property could be added to the Entity because not a single dependency is met!
                                    std::cerr
                                            << "[JsonPropertyContainerPrefab::parse \"properties\"] Error during adding properties! Dependencies could not be met!"
                                            << std::endl;
                                    break;
                                }
                            }
                        });
            }

            C * create(const VariableRegister & variableRegister) override {
                C * c = nullptr;

                // TODO: Agree on global Allocator for PropertyContainers!!!
                c = new C();

                addMyContentTo(*c, variableRegister);
                return c;
            }

            void addMyContentTo(C &c, const VariableRegister & variableRegister) override {
                // Compose given variables with the predefined ones.
                // Therefore, copy the given VariableRegister, such that duplicates
                // are override with the custom variables.
                VariableRegister composedVariableRegister = variableRegister;
                composedVariableRegister.insert(PropertyContainerPrefab<C>::PreDefinedVariables.begin(),
                                                PropertyContainerPrefab<C>::PreDefinedVariables.end());

                std::vector<std::string> parseOrder = {
                        "Inherits",
                        "Properties"
                };

                for (const std::string & name : parseOrder) {
                    if (rootNode->count(name) > 0) {
                        parse(*rootNode.get(), name, c, composedVariableRegister);
                    }
                }

                for (auto &el : rootNode->items()) {
                    if (!Util::vectorContains(parseOrder, el.key())) {
                        parse(*rootNode.get(), el.key(), c, composedVariableRegister);
                    }
                }
            }

            const Path & getPath() {
                return path;
            }
        };

        template<typename C>
        JsonProperyContainerPrefabElementParserRegister<C> JsonPropertyContainerPrefab<C>::Parsers;
    }
}

#endif //PAXENGINE3_JSONPROPERTYCONTAINERPREFAB_H
