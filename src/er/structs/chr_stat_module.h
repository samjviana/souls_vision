//
// Created by PC-SAMUEL on 24/11/2024.
//

#ifndef SOULS_VISION_CHR_STAT_MODULE_H
#define SOULS_VISION_CHR_STAT_MODULE_H

#include <cstdint>

namespace structs {

class ChrStatModule {
public:
    char pad_0x0000[312];   //0x0000
    int32_t hp;             //0x0138
    int32_t maxHp;          //0x013C
    int32_t secMaxHp;       //0x0140
    int32_t baseHp;         //0x0144
    int32_t fp;             //0x0148
    int32_t maxFp;          //0x014C
    int32_t baseFp;         //0x0150
    int32_t stamina;        //0x0154
    int32_t maxStamina;     //0x0158
    int32_t baseStamina;    //0x015C
};

} // structs

#endif //SOULS_VISION_CHR_STAT_MODULE_H
