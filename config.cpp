//
// Created by PC-SAMUEL on 23/11/2024.
//

#include "config.h"
#include "globals.h"

namespace souls_vision {

ComponentVisibility Config::componentVisibility;
BarSettings Config::statBarSettings;
ImVec2 Config::bestEffectIconSize = ImVec2(39, 33);
ImVec2 Config::dmgTypeIconSize = ImVec2(30, 30);
ImVec2 Config::effectBarIconSize = ImVec2(56, 48);
int Config::bestEffects = 3;
int Config::statBarSpacing = 0;
float Config::fontSize = 18.0f;
float Config::opacity;
bool Config::debug = false;
bool Config::dragOverlay = false;
bool Config::configUpdated = false;
bool Config::opacityUpdated = false;
bool Config::fontSizeUpdated = false;

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

        if (!configJson.contains("fontSize")) {
            configJson["fontSize"] = 18.0f;
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
            configJson["bestEffectIconSize"] = 39;
            updated = true;
        }

        if (!configJson.contains("dmgTypeIconSize")) {
            configJson["dmgTypeIconSize"] = 30;
            updated = true;
        }

        if (!configJson.contains("componentVisibility") && !configJson.contains("barVisibility")) {
            configJson["componentVisibility"] = {
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
                    {"madness", true},
                    {"bestEffects", true},
                    {"immuneEffects", true},
                    {"dmgTypes", true},
                    {"neutralDmgTypes", false}
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
    configJson["fontSize"] = fontSize;
    configJson["opacity"] = opacity;
    configJson["bestEffects"] = bestEffects;

    configJson["componentVisibility"] = {
        {"hp",          componentVisibility.hp},
        {"fp",          componentVisibility.fp},
        {"stamina",     componentVisibility.stamina},
        {"stagger",     componentVisibility.stagger},
        {"poison",      componentVisibility.poison},
        {"scarletRot",  componentVisibility.scarletRot},
        {"hemorrhage",  componentVisibility.hemorrhage},
        {"deathBlight", componentVisibility.deathBlight},
        {"frostbite",   componentVisibility.frostbite},
        {"sleep",       componentVisibility.sleep},
        {"madness",     componentVisibility.madness},
        {"bestEffects", componentVisibility.bestEffects},
        {"immuneEffects", componentVisibility.immuneEffects},
        {"dmgTypes", componentVisibility.dmgTypes},
        {"neutralDmgTypes", componentVisibility.neutralDmgTypes}
    };

    if (configJson.contains("barVisibility")) {
        Logger::Info("Updating old config format to new format.");
        Logger::Info("Fields: barVisibility -> componentVisibility");
        configJson.erase("barVisibility");
    }

    configJson["statBar"]["position"]["x"] = statBarSettings.position.x;
    configJson["statBar"]["position"]["y"] = statBarSettings.position.y;
    configJson["statBar"]["size"]["width"] = statBarSettings.size.x;
    configJson["statBar"]["size"]["height"] = statBarSettings.size.y;
    configJson["statBar"]["hideText"] = statBarSettings.hideText;

    configJson["bestEffectIconSize"] = bestEffectIconSize.x;
    configJson["dmgTypeIconSize"] = dmgTypeIconSize.x;
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
        if (configJson["fontSize"] != fontSize) {
            fontSizeUpdated = true;
        }
        fontSize = configJson["fontSize"];
        if (configJson["opacity"] != opacity) {
            opacityUpdated = true;
        }
        opacity = configJson["opacity"];
        bestEffects = configJson["bestEffects"];
        bestEffectIconSize.x = configJson["bestEffectIconSize"];
        bestEffectIconSize.y = bestEffectIconSize.x * 0.85f;
        dmgTypeIconSize.x = configJson["dmgTypeIconSize"];
        dmgTypeIconSize.y = dmgTypeIconSize.x;
        statBarSpacing = configJson["statBarSpacing"];

        componentVisibility = LoadComponentVisibility(configJson);

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
        configJson["fontSize"] = 18.0f;
        configJson["opacity"] = 0.9f;
        configJson["bestEffects"] = 3;
        configJson["bestEffectIconSize"] = 39;
        configJson["dmgTypeIconSize"] = 30;
        configJson["statBarSpacing"] = 0;

        configJson["componentVisibility"] = {
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
            {"madness", true},
            {"bestEffects", true},
            {"immuneEffects", true},
            {"dmgTypes", true},
            {"neutralDmgTypes", false}
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

ComponentVisibility Config::LoadComponentVisibility(const nlohmann::json &configJson) {
    if (!configJson.contains("componentVisibility") && !configJson.contains("barVisibility")) {
        return {
            true, true, true, true, true, true, true, true, true, true, true, true, true, true, false
        };
    }

    if (configJson.contains("barVisibility") && !configJson.contains("componentVisibility")) {
        return {
            configJson["barVisibility"]["hp"],
            configJson["barVisibility"]["fp"],
            configJson["barVisibility"]["stamina"],
            configJson["barVisibility"]["stagger"],
            configJson["barVisibility"]["poison"],
            configJson["barVisibility"]["scarletRot"],
            configJson["barVisibility"]["hemorrhage"],
            configJson["barVisibility"]["deathBlight"],
            configJson["barVisibility"]["frostbite"],
            configJson["barVisibility"]["sleep"],
            configJson["barVisibility"]["madness"],
            true,
            true,
            true,
            false
        };
    }

    return {
        configJson["componentVisibility"]["hp"],
        configJson["componentVisibility"]["fp"],
        configJson["componentVisibility"]["stamina"],
        configJson["componentVisibility"]["stagger"],
        configJson["componentVisibility"]["poison"],
        configJson["componentVisibility"]["scarletRot"],
        configJson["componentVisibility"]["hemorrhage"],
        configJson["componentVisibility"]["deathBlight"],
        configJson["componentVisibility"]["frostbite"],
        configJson["componentVisibility"]["sleep"],
        configJson["componentVisibility"]["madness"],
        configJson["componentVisibility"]["bestEffects"],
        configJson["componentVisibility"]["immuneEffects"],
        configJson["componentVisibility"]["dmgTypes"],
        configJson["componentVisibility"]["neutralDmgTypes"]
    };
}

} // souls_vision