//
// Created by Bittner on 05.12.2019.
//

#include "Cheese.h"

namespace PAX::Examples {
    PAX_PROPERTY_IMPL(Cheese)

    std::string Cheese::yummy() {
        return "Cheesyyyyyyy";
    }

    void Cheese::baked(struct BakedEvent & e) {
        PAX_LOG(PAX::Log::Level::Info, "Cheese turns brown.");
    }

    void Cheese::attached(class Pizza & pizza) {
        pizza.getEventService().add<BakedEvent, Cheese, &Cheese::baked>(this);
    }

    void Cheese::detached(class Pizza & pizza) {
        pizza.getEventService().remove<BakedEvent, Cheese, &Cheese::baked>(this);
    }
}
