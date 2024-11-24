//
// Created by PC-SAMUEL on 22/11/2024.
//

#ifndef SOULS_VISION_CS_MENU_MAN_IMP_H
#define SOULS_VISION_CS_MENU_MAN_IMP_H

#include "loading_screen_data.h"

namespace structs {

class CSMenuManImp {
public:
    char pad_0x0000[1840];                  //0x0000
    LoadingScreenData loadingScreenData;    //0x0730
};

} // structs

#endif //SOULS_VISION_CS_MENU_MAN_IMP_H
