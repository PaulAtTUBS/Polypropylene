//
// Created by Bittner on 06.12.2019.
//

#ifndef PROTEUS_TOMATOSAUCE_H
#define PROTEUS_TOMATOSAUCE_H

#include "../Topping.h"

namespace Proteus::Examples {
    class TomatoSauce : public Topping {
        PAX_PROPERTY(TomatoSauce, PAX_PROPERTY_IS_CONCRETE)
        PAX_PROPERTY_DERIVES(Topping)
        PAX_PROPERTY_IS_SINGLE

        int scoville = 0;

    public:
        std::string yummy() override;
    };
}

#endif //PROTEUS_TOMATOSAUCE_H
