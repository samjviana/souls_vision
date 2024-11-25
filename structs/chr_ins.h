//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_CHR_INS_H
#define SOULS_VISION_CHR_INS_H

#include <cstdint>
#include "chr_module_bag.h"

namespace structs {

class ChrIns {
public:
    char pad_0000[8];           //0x0000
    uint64_t handle;            //0x0008
    char pad_0010[88];          //0x0010
    int32_t chrType;            //0x0068
    char pad_006C[292];         //0x006C
    ChrModuleBag* moduleBag;    //0x0190
    char pad_0198[1304];        //0x0198
    uint64_t targetHandle;      //0x06B0
};

} // structs

#endif //SOULS_VISION_CHR_INS_H
