//
// Created by Paul on 08.05.2017.
//

#ifndef POLYPROPYLENE_EVENTSERVICE_H
#define POLYPROPYLENE_EVENTSERVICE_H

#include "Delegate.h"
#include <polypropylene/reflection/TypeMap.h>
#include <polypropylene/stdutils/CollectionUtils.h>

namespace PAX {
    class EventService {
    protected:
        EventService *_parent = nullptr;
        TypeMap<void*> _listeners;

        template<typename EventClass, class T, void (T::*Method)(EventClass&)>
        static void invoke(void* callee, EventClass& event) {
            T* object = static_cast<T*>(callee);
            (object->*Method)(event);
        };

    public:
        EventService() = default;
        EventService(const EventService & other) = delete;
        EventService(const EventService && other) = delete;
        EventService & operator=(const EventService & other) = delete;
        EventService & operator=(const EventService && other) = delete;

        void setParent(EventService *parent);
        EventService* getParent();

#define PAX_ES_DELEGATE Delegate<EventClass&>
#define PAX_ES_MAP_VALUES std::vector<PAX_ES_DELEGATE>

        template<typename EventClass, typename Listener, void (Listener::*Method)(EventClass&)>
        void add(Listener* listener) {
            PAX_ES_MAP_VALUES* listenerList;

            const auto & listenerListIt = _listeners.find(paxtypeid(EventClass));
            if (listenerListIt != _listeners.end())
                listenerList = static_cast<PAX_ES_MAP_VALUES*>(listenerListIt->second);
            else {
                listenerList = new PAX_ES_MAP_VALUES;
                _listeners[paxtypeid(EventClass)] = listenerList;
            }

            PAX_ES_DELEGATE delegate(listener, &invoke<EventClass, Listener, Method>);
            if (!Util::vectorContains(*listenerList, delegate)) {
                listenerList->emplace_back(delegate);
            }
        }

        template<typename EventClass, typename Listener, void (Listener::*Method)(EventClass&)>
        bool remove(Listener *listener) {
            const auto & listenerListIt = _listeners.find(paxtypeid(EventClass));
            if (listenerListIt != _listeners.end()) {
                PAX_ES_MAP_VALUES* vec = static_cast<PAX_ES_MAP_VALUES*>(listenerListIt->second);
                return PAX::Util::removeFromVector(*vec, PAX_ES_DELEGATE(listener, &invoke<EventClass, Listener, Method>));
            }

            return false;
        }

        template<typename EventClass>
        void operator()(EventClass& event) {
            fire(event);
        }

        template<typename EventClass>
        void fire(EventClass& event) {
            const auto & listener = _listeners.find(paxtypeid(EventClass));
            if (listener != _listeners.end()) {
                PAX_ES_MAP_VALUES *values = static_cast<PAX_ES_MAP_VALUES *>(listener->second);
                for (PAX_ES_DELEGATE &delegate : *values) {
                    delegate.method(delegate.callee, event);

                    if (event.isConsumed()) {
                        break;
                    }
                }
            }

            if (_parent)
                _parent->fire(event);
        }

#undef PAX_ES_DELEGATE
#undef PAX_ES_MAP_VALUES
    };
}

#endif //POLYPROPYLENE_EVENTSERVICE_H
