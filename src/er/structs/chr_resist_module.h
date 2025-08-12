//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_CHR_RESIST_MODULE_H
#define SOULS_VISION_CHR_RESIST_MODULE_H

#include <cstdint>

namespace structs {

class ChrResistModule {
public:
    char pad_0x0000[16];            //0x0000
    int32_t poisonResist;          //0x0010
    int32_t scarletRotResist;      //0x0014
    int32_t hemorrhageResist;      //0x0018
    int32_t deathBlightResist;     //0x001C
    int32_t frostbiteResist;       //0x0020
    int32_t sleepResist;           //0x0024
    int32_t madnessResist;         //0x0028
    int32_t maxPoisonResist;       //0x002C
    int32_t maxScarletRotResist;   //0x0030
    int32_t maxHemorrhageResist;   //0x0034
    int32_t maxDeathBlightResist;  //0x0038
    int32_t maxFrostbiteResist;    //0x003C
    int32_t maxSleepResist;        //0x0040
    int32_t maxMadnessResist;      //0x0044
};

} // structs

#endif //SOULS_VISION_CHR_RESIST_MODULE_H
