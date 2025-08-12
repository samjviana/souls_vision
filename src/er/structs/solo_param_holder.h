//
// Created by PC-SAMUEL on 27/11/2024.
//

#ifndef SOULS_VISION_SOLO_PARAM_HOLDER_H
#define SOULS_VISION_SOLO_PARAM_HOLDER_H

#include "param_res_cap.h"

namespace structs {

class SoloParamHolder {
    bool paramLoaded;           //0x0000
    char pad_0x0001[7];         //0x0001
    ParamResCap* paramResCap;   //0x0008
    char pad_0x0010[56];        //0x0010
};

} // structs

#endif //SOULS_VISION_SOLO_PARAM_HOLDER_H
