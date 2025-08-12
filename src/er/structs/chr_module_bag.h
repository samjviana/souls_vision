//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_CHR_MODULE_BAG_H
#define SOULS_VISION_CHR_MODULE_BAG_H

#include "chr_stat_module.h"
#include "chr_resist_module.h"
#include "chr_super_armor_module.h"

namespace structs {

class ChrModuleBag {
public:
    ChrStatModule* statModule;              // 0x0000
    char pad0x0008[24];                     // 0x0008
    ChrResistModule* resistModule;          // 0x0020
    char pad0x0028[24];                     // 0x0028
    ChrSuperArmorModule* superArmorModule;  // 0x0040
};

} // structs

#endif //SOULS_VISION_CHR_MODULE_BAG_H
