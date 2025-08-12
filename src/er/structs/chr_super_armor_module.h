//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_CHR_SUPER_ARMOR_MODULE_H
#define SOULS_VISION_CHR_SUPER_ARMOR_MODULE_H

namespace structs {

class ChrSuperArmorModule {
public:
    char pad0x0000[16];  // 0x0000
    float stagger;       // 0x0010
    float maxStagger;    // 0x0014
    char pad0x0018[4];   // 0x0018
    float resetTimer;    // 0x001C
};

} // structs

#endif //SOULS_VISION_CHR_SUPER_ARMOR_MODULE_H
