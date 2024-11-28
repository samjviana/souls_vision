//
// Created by PC-SAMUEL on 23/11/2024.
//

#include "config.h"
#include "globals.h"

namespace souls_vision {

BarVisibility Config::barVisibility;
BarSettings Config::statBarSettings;
ImVec2 Config::bestEffectIconSize = ImVec2(33, 28);
ImVec2 Config::effectBarIconSize = ImVec2(56, 48);
int Config::bestEffects = 3;
int Config::statBarSpacing = 0;
float Config::opacity;
bool Config::debug = false;
bool Config::dragOverlay = false;
bool Config::configUpdated = false;

bool Config::CheckConfig(const std::string& configFilePath) {
    try {
        if (!std::filesystem::exists(configFilePath)) {
            Logger::Info("Config file not found. Creating a new one...");
            CreateConfig(configFilePath);
            return true;
        }

        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open sv_config.json");
        }

        nlohmann::json configJson;
        configFile >> configJson;
        configFile.close();

        bool updated = false;

        if (!configJson.contains("debug")) {
            configJson["debug"] = false;
            updated = true;
        }

        if (!configJson.contains("dragOverlay")) {
            configJson["dragOverlay"] = false;
            updated = true;
        }

        if (!configJson.contains("opacity")) {
            configJson["opacity"] = 1.0f;
            updated = true;
        }

        if (!configJson.contains("bestEffects")) {
            configJson["bestEffects"] = 3;
            updated = true;
        }

        if (!configJson.contains("statBarSpacing")) {
            configJson["statBarSpacing"] = 0;
            updated = true;
        }

        if (!configJson.contains("bestEffectIconSize")) {
            configJson["bestEffectIconSize"] = 33;
            updated = true;
        }

        if (!configJson.contains("barVisibility")) {
            configJson["barVisibility"] = {
                    {"hp", true},
                    {"fp", true},
                    {"stamina", true},
                    {"stagger", true},
                    {"poison", true},
                    {"scarletRot", true},
                    {"hemorrhage", true},
                    {"deathBlight", true},
                    {"frostbite", true},
                    {"sleep", true},
                    {"madness", true}
            };
            updated = true;
        }

        if (!configJson.contains("statBar")) {
            configJson["statBar"] = {
                    {"position", {{"x", gGameWindowSize.width - 555 - 5}, {"y", 10}}},
                    {"size", {{"width", 555}, {"height", 40}}},
                    {"hideText", false}
            };
            updated = true;
        } else {
            if (!configJson["statBar"].contains("position")) {
                configJson["statBar"]["position"] = {{"x", gGameWindowSize.width - 555 - 5}, {"y", 10}};
                updated = true;
            }
            if (!configJson["statBar"].contains("size")) {
                configJson["statBar"]["size"] = {{"width", 555}, {"height", 40}};
                updated = true;
            }
            if (!configJson["statBar"].contains("hideText")) {
                configJson["statBar"]["hideText"] = false;
                updated = true;
            }
        }

        if (updated) {
            Logger::Info("Config file was missing fields. Updating...");
            std::ofstream outFile(configFilePath);
            if (!outFile.is_open()) {
                throw std::runtime_error("Failed to write to sv_config.json");
            }
            outFile << configJson.dump(4);
            outFile.close();
            return true;
        }

        return true;
    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::CheckConfig - Error: ") + e.what());
        return false;
    }
}

void Config::SaveConfig(const std::string& configFilePath) {
    std::ofstream configFile(configFilePath);
    if (!configFile.is_open()) {
        Logger::Error("Failed to open sv_config.json for writing.");
        return;
    }
    nlohmann::json configJson;

    configJson["debug"] = debug;
    configJson["dragOverlay"] = dragOverlay;
    configJson["opacity"] = opacity;
    configJson["bestEffects"] = bestEffects;

    configJson["barVisibility"] = {
        {"hp", barVisibility.hp},
        {"fp", barVisibility.fp},
        {"stamina", barVisibility.stamina},
        {"stagger", barVisibility.stagger},
        {"poison", barVisibility.poison},
        {"scarletRot", barVisibility.scarletRot},
        {"hemorrhage", barVisibility.hemorrhage},
        {"deathBlight", barVisibility.deathBlight},
        {"frostbite", barVisibility.frostbite},
        {"sleep", barVisibility.sleep},
        {"madness", barVisibility.madness}
    };

    configJson["statBar"]["position"]["x"] = statBarSettings.position.x;
    configJson["statBar"]["position"]["y"] = statBarSettings.position.y;
    configJson["statBar"]["size"]["width"] = statBarSettings.size.x;
    configJson["statBar"]["size"]["height"] = statBarSettings.size.y;
    configJson["statBar"]["hideText"] = statBarSettings.hideText;

    configJson["bestEffectIconSize"] = bestEffectIconSize.x;
    configJson["statBarSpacing"] = statBarSpacing;

    configFile << configJson.dump(4);
    configFile.close();
}

void Config::LoadConfig(const std::string& configFilePath) {
    if (!CheckConfig(configFilePath)) {
        Logger::Warning("Failed to validate config file.");
        return;
    }

    try {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open sv_config.json");
        }

        nlohmann::json configJson;
        configFile >> configJson;

        debug = configJson["debug"];
        dragOverlay = configJson["dragOverlay"];
        opacity = configJson["opacity"];
        bestEffects = configJson["bestEffects"];
        bestEffectIconSize.x = configJson["bestEffectIconSize"];
        bestEffectIconSize.y = bestEffectIconSize.x * 0.85f;
        statBarSpacing = configJson["statBarSpacing"];

        barVisibility.hp = configJson["barVisibility"]["hp"];
        barVisibility.fp = configJson["barVisibility"]["fp"];
        barVisibility.stamina = configJson["barVisibility"]["stamina"];
        barVisibility.stagger = configJson["barVisibility"]["stagger"];
        barVisibility.poison = configJson["barVisibility"]["poison"];
        barVisibility.scarletRot = configJson["barVisibility"]["scarletRot"];
        barVisibility.hemorrhage = configJson["barVisibility"]["hemorrhage"];
        barVisibility.deathBlight = configJson["barVisibility"]["deathBlight"];
        barVisibility.frostbite = configJson["barVisibility"]["frostbite"];
        barVisibility.sleep = configJson["barVisibility"]["sleep"];
        barVisibility.madness = configJson["barVisibility"]["madness"];

        statBarSettings.position = ImVec2(configJson["statBar"]["position"]["x"], configJson["statBar"]["position"]["y"]);
        statBarSettings.size = ImVec2(configJson["statBar"]["size"]["width"], configJson["statBar"]["size"]["height"]);
        statBarSettings.hideText = configJson["statBar"]["hideText"];

        float iconWidth = statBarSettings.size.y * 1.70f;
        effectBarIconSize = ImVec2(iconWidth, iconWidth * 0.85f);

    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::LoadConfig - Error: ") + e.what());
    }

    Logger::Info("Config loaded successfully.");
}

void Config::CreateConfig(const std::string &configFilePath) {
    Size barSize = {600, 40};

    try {
        nlohmann::json configJson;

        configJson["debug"] = false;
        configJson["dragOverlay"] = false;
        configJson["opacity"] = 0.9f;
        configJson["bestEffects"] = 3;
        configJson["bestEffectIconSize"] = 33;
        configJson["statBarSpacing"] = 0;

        configJson["barVisibility"] = {
            {"hp", true},
            {"fp", true},
            {"stamina", true},
            {"stagger", true},
            {"poison", true},
            {"scarletRot", true},
            {"hemorrhage", true},
            {"deathBlight", true},
            {"frostbite", true},
            {"sleep", true},
            {"madness", true}
        };

        configJson["statBar"]["position"]["x"] = gGameWindowSize.width - barSize.width - 5;
        configJson["statBar"]["position"]["y"] = 10;
        configJson["statBar"]["size"]["width"] = barSize.width;
        configJson["statBar"]["size"]["height"] = barSize.height;
        configJson["statBar"]["hideText"] = false;

        configJson["effectBar"]["position"]["x"] = gGameWindowSize.width - barSize.width - 3;
        configJson["effectBar"]["position"]["y"] = 10;
        configJson["effectBar"]["size"]["width"] = barSize.width;
        configJson["effectBar"]["size"]["height"] = barSize.height;
        configJson["effectBar"]["hideText"] = false;

        std::ofstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to create sv_config.json");
        }

        configFile << configJson.dump(4);
        configFile.close();

    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::CreateConfig - Error: ") + e.what());
    }
}

} // souls_vision