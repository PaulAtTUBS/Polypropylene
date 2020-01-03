//
// Created by Bittner on 06.12.2019.
//

#include <polypropylene/reflection/ClassMetadata.h>
#include "TomatoSauce.h"

namespace PAX::Examples {
    PAX_PROPERTY_INIT(TomatoSauce) {}

    TomatoSauce::TomatoSauce() = default;
    TomatoSauce::TomatoSauce(int scoville) : scoville(scoville) { init(); }

    ClassMetadata TomatoSauce::getMetadata() {
        ClassMetadata m = Super::getMetadata();
        m.add(paxfieldof(scoville)).flags = Field::IsMandatory;
        return m;
    }

    std::string TomatoSauce::yummy() {
        return std::to_string(scoville) + " scoville tomato sauce";
    }
}