//
// Created by PC-SAMUEL on 27/11/2024.
//

#ifndef SOULS_VISION_PARAM_ROW_H
#define SOULS_VISION_PARAM_ROW_H

#include <cstdint>

namespace structs {

class ParamRow {
public:
    uint32_t rowId;         // 0x0000
    char pad_0x0004[4];     //0x0004
    uint32_t paramOffset;   //0x0008
    char pad_0x000C[4];     //0x000C
    uint32_t stringOffset;  //0x0010
    char pad_0x0014[4];     //0x0014
};

} // structs

#endif //SOULS_VISION_PARAM_ROW_H
