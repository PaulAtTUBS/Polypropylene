//
// Created by Bittner on 06.12.2019.
//

#include "Mozzarella.h"

namespace PAX::Examples {
    PAX_PROPERTY_INIT(Mozzarella) {}

    std::string Mozzarella::yummy() {
        return Super::yummy() + " Mozzarella";
    }
}