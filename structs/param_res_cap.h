//
// Created by PC-SAMUEL on 27/11/2024.
//

#ifndef SOULS_VISION_PARAM_RES_CAP_H
#define SOULS_VISION_PARAM_RES_CAP_H

#include "fd4_basic_hash_string.h"
#include "param_data.h"

namespace structs {

class ParamResCap {
public:
    char pad_0000[8];               //0x0000
    FD4BasicHashString hashString;  //0x0008
    char pad_0048[56];              //0x0048
    ParamData* paramData;           //0x0080
};

} // structs

#endif //SOULS_VISION_PARAM_RES_CAP_H
