//
// Created by PC-SAMUEL on 27/11/2024.
//

#ifndef SOULS_VISION_SOLO_PARAM_REPOSITORY_IMP_H
#define SOULS_VISION_SOLO_PARAM_REPOSITORY_IMP_H

#include "fd4_basic_hash_string.h"
#include "solo_param_holder.h"

namespace structs {

class SoloParamRepositoryImp {
public:
    char pad_0x0000[8];                 //0x0000
    FD4BasicHashString hashString;      //0x0008
    char pad_0x0048[56];                //0x0048
    SoloParamHolder paramHolder[194];   //0x0080
};

} // structs

#endif //SOULS_VISION_SOLO_PARAM_REPOSITORY_IMP_H
