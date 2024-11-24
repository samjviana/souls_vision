//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_ENTRY_H
#define SOULS_VISION_ENTRY_H

#include "chr_ins.h"

namespace structs {

class Entry {
public:
    ChrIns* chrIns;     //0x0000
    char pad_0008[8];   //0x0008
};

} // structs

#endif //SOULS_VISION_ENTRY_H
