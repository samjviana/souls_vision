//
// Created by PC-SAMUEL on 27/11/2024.
//

#ifndef SOULS_VISION_FD4_BASIC_HASH_STRING_H
#define SOULS_VISION_FD4_BASIC_HASH_STRING_H

namespace structs {

class FD4BasicHashString {
public:
    char pad_0x0000[16];    //0x0000
    wchar_t* hashString;    //0x0010
    char pad_0x0018[8];     //0x0018
    int length;             //0x0020
    char pad_0x0024[28];    //0x0024
};

} // structs

#endif //SOULS_VISION_FD4_BASIC_HASH_STRING_H
