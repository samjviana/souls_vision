//
// Created by PC-SAMUEL on 27/11/2024.
//

#ifndef SOULS_VISION_PARAM_HEADER_H
#define SOULS_VISION_PARAM_HEADER_H

#include <cstdint>
#include "param_row.h"

namespace structs {

class ParamHeader {
public:
    uint32_t stringOffset;      // 0x0000
    char pad_0x0000[6];         // 0x0004
    uint16_t rowCount;          // 0x000A
    char pad_0x000C[4];         // 0x000C
    uint64_t paramTypeOffset;   // 0x0010
    char pad_0x0018[24];        // 0x0018
    uint32_t dataOffset;        // 0x0030
    char pad_0x0034[12];        // 0x0034
    ParamRow paramRows;         // 0x0040
};

} // structs

#endif //SOULS_VISION_PARAM_HEADER_H
