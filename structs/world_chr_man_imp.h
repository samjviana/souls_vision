//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_WORLD_CHR_MAN_IMP_H
#define SOULS_VISION_WORLD_CHR_MAN_IMP_H

#include "chr_ins.h"
#include "entry.h"

namespace structs {

class WorldChrManImp {
public:
    char pad_0x0000[69368]; // 0x0000
    Entry(*playerArray)[6]; // 0x10EF8
};

} // structs

#endif //SOULS_VISION_WORLD_CHR_MAN_IMP_H
