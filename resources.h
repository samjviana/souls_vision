//
// Created by PC-SAMUEL on 28/11/2024.
//

#ifndef SOULS_VISION_RESOURCES_H
#define SOULS_VISION_RESOURCES_H

#define RT_PNG 101

#define IDR_AGMENA_W1G_FONT 101

#ifndef RC_INVOKED
struct TextureResource
{
    const char* file_name;
    const char* resource_id;
};

const TextureResource TEXTURES[] = {
        {"Bar.png", "IDR_PNG_BAR"},
        {"BarBG.png", "IDR_PNG_BARBG"},
        {"BarEdge.png", "IDR_PNG_BAREDGE"},
        {"BarEdge2.png", "IDR_PNG_BAREDGE2"},
        {"Blue.png", "IDR_PNG_BLUE"},
        {"BuddyWaku.png", "IDR_PNG_BUDDYWAKU"},
        {"ConditionWaku.png", "IDR_PNG_CONDITIONWAKU"},
        {"DeathBlight.png", "IDR_PNG_DEATHBLIGHT"},
        {"Fire.png", "IDR_PNG_FIRE"},
        {"Frostbite.png", "IDR_PNG_FROSTBITE"},
        {"Green.png", "IDR_PNG_GREEN"},
        {"GreenArrow.png", "IDR_PNG_GREENARROW"},
        {"Hemorrhage.png", "IDR_PNG_HEMORRHAGE"},
        {"Holy.png", "IDR_PNG_HOLY"},
        {"Lightning.png", "IDR_PNG_LIGHTNING"},
        {"Madness.png", "IDR_PNG_MADNESS"},
        {"Magic.png", "IDR_PNG_MAGIC"},
        {"Poison.png", "IDR_PNG_POISON"},
        {"Red.png", "IDR_PNG_RED"},
        {"RedArrow.png", "IDR_PNG_REDARROW"},
        {"ScarletRot.png", "IDR_PNG_SCARLETROT"},
        {"Sleep.png", "IDR_PNG_SLEEP"},
        {"Yellow.png", "IDR_PNG_YELLOW"}
};
#endif // if RC_INVOKED

#endif //SOULS_VISION_RESOURCES_H
