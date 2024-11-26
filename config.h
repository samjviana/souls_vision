//
// Created by PC-SAMUEL on 23/11/2024.
//

#ifndef SOULS_VISION_CONFIG_H
#define SOULS_VISION_CONFIG_H

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <imgui.h>
#include "logger.h"
#include "bar_renderer.h"

namespace souls_vision {

class Config {
public:
    static BarVisibility barVisibility;
    static BarSettings statBarSettings;
    static BarSettings effectBarSettings;
    static ImVec2 effectBarIconSize;
    static int bestEffects;
    static float opacity;
    static bool debug;

    static bool CheckConfig(const std::string& configFilePath);
    static void LoadConfig(const std::string& configFilePath);

private:
    static void CreateConfig(const std::string& configFilePath);
};

} // souls_vision

#endif //SOULS_VISION_CONFIG_H
