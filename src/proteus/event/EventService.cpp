//
// Created by paul on 01.06.17.
//

#include <proteus/event/EventService.h>

namespace PAX {
    EventService* EventService::getParent() {
        return _parent;
    }

    void EventService::setParent(EventService *parent) {
        _parent = parent;
    }
}