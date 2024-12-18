//
// Created by PC-SAMUEL on 23/11/2024.
//

#include "config.h"
#include "globals.h"

namespace souls_vision {

Components Config::components;
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
bool Config::hideBlightMadness = false;
int Config::maxEffectBars = 7;

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

        if (!configJson.contains("hideBlightMadness")) {
            configJson["hideBlightMadness"] = false;
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

        if (!configJson.contains("maxEffectBars")) {
            configJson["maxEffectBars"] = 7;
            updated = true;
        }

        if (!configJson.contains("components") && !configJson.contains("barVisibility")) {
            configJson["components"] = {
                    {"hp", {{"visible", true}, {"hideText", false}}},
                    {"fp", {{"visible", true}, {"hideText", false}}},
                    {"stamina", {{"visible", true}, {"hideText", false}}},
                    {"stagger", {{"visible", true}, {"hideText", false}}},
                    {"poison", {{"visible", true}, {"hideText", false}}},
                    {"scarletRot", {{"visible", true}, {"hideText", false}}},
                    {"hemorrhage", {{"visible", true}, {"hideText", false}}},
                    {"deathBlight", {{"visible", true}, {"hideText", false}}},
                    {"frostbite", {{"visible", true}, {"hideText", false}}},
                    {"sleep", {{"visible", true}, {"hideText", false}}},
                    {"madness", {{"visible", true}, {"hideText", false}}},
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
                    {"size", {{"width", 555}, {"height", 40}}}
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
    configJson["maxEffectBars"] = maxEffectBars;

    configJson["components"] = {
        {"hp",              {{"visible", components.hp.visible},      {"hideText", components.hp.hideText}}},
        {"fp",              {{"visible", components.fp.visible},      {"hideText", components.fp.hideText}}},
        {"stamina",         {{"visible", components.stamina.visible}, {"hideText", components.stamina.hideText}}},
        {"stagger",         {{"visible", components.stagger.visible}, {"hideText", components.stagger.hideText}}},
        {"poison",          {{"visible", components.poison.visible},  {"hideText", components.poison.hideText}}},
        {"scarletRot",      {{"visible", components.scarletRot.visible}, {"hideText", components.scarletRot.hideText}}},
        {"hemorrhage",      {{"visible", components.hemorrhage.visible}, {"hideText", components.hemorrhage.hideText}}},
        {"deathBlight",     {{"visible", components.deathBlight.visible}, {"hideText", components.deathBlight.hideText}}},
        {"frostbite",       {{"visible", components.frostbite.visible}, {"hideText", components.frostbite.hideText}}},
        {"sleep",           {{"visible", components.sleep.visible}, {"hideText", components.sleep.hideText}}},
        {"madness",         {{"visible", components.madness.visible}, {"hideText", components.madness.hideText}}},
        {"bestEffects",     components.bestEffects},
        {"immuneEffects",   components.immuneEffects},
        {"dmgTypes",        components.dmgTypes},
        {"neutralDmgTypes", components.neutralDmgTypes}
    };

    if (configJson.contains("barVisibility")) {
        Logger::Info("Updating old config format to new format.");
        Logger::Info("Fields: barVisibility -> components");
        configJson.erase("barVisibility");
    }

    configJson["statBar"]["position"]["x"] = statBarSettings.position.x;
    configJson["statBar"]["position"]["y"] = statBarSettings.position.y;
    configJson["statBar"]["size"]["width"] = statBarSettings.size.x;
    configJson["statBar"]["size"]["height"] = statBarSettings.size.y;

    configJson["bestEffectIconSize"] = bestEffectIconSize.x;
    configJson["dmgTypeIconSize"] = dmgTypeIconSize.x;
    configJson["statBarSpacing"] = statBarSpacing;
    configJson["hideBlightMadness"] = hideBlightMadness;

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
        hideBlightMadness = configJson["hideBlightMadness"];
        maxEffectBars = configJson["maxEffectBars"];

        components = LoadComponentVisibility(configJson);

        statBarSettings.position = ImVec2(configJson["statBar"]["position"]["x"], configJson["statBar"]["position"]["y"]);
        statBarSettings.size = ImVec2(configJson["statBar"]["size"]["width"], configJson["statBar"]["size"]["height"]);

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
        configJson["hideBlightMadness"] = false;
        configJson["fontSize"] = 18.0f;
        configJson["opacity"] = 0.9f;
        configJson["bestEffects"] = 3;
        configJson["bestEffectIconSize"] = 39;
        configJson["dmgTypeIconSize"] = 30;
        configJson["statBarSpacing"] = 0;
        configJson["maxEffectBars"] = 7;

        configJson["components"] = {
            {"hp", {{"visible", true}, {"hideText", false}}},
            {"fp", {{"visible", true}, {"hideText", false}}},
            {"stamina", {{"visible", true}, {"hideText", false}}},
            {"stagger", {{"visible", true}, {"hideText", false}}},
            {"poison", {{"visible", true}, {"hideText", false}}},
            {"scarletRot", {{"visible", true}, {"hideText", false}}},
            {"hemorrhage", {{"visible", true}, {"hideText", false}}},
            {"deathBlight", {{"visible", true}, {"hideText", false}}},
            {"frostbite", {{"visible", true}, {"hideText", false}}},
            {"sleep", {{"visible", true}, {"hideText", false}}},
            {"madness", {{"visible", true}, {"hideText", false}}},
            {"bestEffects", true},
            {"immuneEffects", true},
            {"dmgTypes", true},
            {"neutralDmgTypes", false}
        };

        configJson["statBar"]["position"]["x"] = gGameWindowSize.width - barSize.width - 5;
        configJson["statBar"]["position"]["y"] = 10;
        configJson["statBar"]["size"]["width"] = barSize.width;
        configJson["statBar"]["size"]["height"] = barSize.height;

        configJson["effectBar"]["position"]["x"] = gGameWindowSize.width - barSize.width - 3;
        configJson["effectBar"]["position"]["y"] = 10;
        configJson["effectBar"]["size"]["width"] = barSize.width;
        configJson["effectBar"]["size"]["height"] = barSize.height;

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

Components Config::LoadComponentVisibility(const nlohmann::json &configJson) {
    if (!configJson.contains("components") && !configJson.contains("barVisibility")) {
        return {
            true, true, true, true, true, true, true, true, true, true, true, true, true, true, false
        };
    }

    if (configJson.contains("barVisibility") && !configJson.contains("components")) {
        return {
            configJson["barVisibility"]["hp"],
            false,
            configJson["barVisibility"]["fp"],
            false,
            configJson["barVisibility"]["stamina"],
            false,
            configJson["barVisibility"]["stagger"],
            false,
            configJson["barVisibility"]["poison"],
            false,
            configJson["barVisibility"]["scarletRot"],
            false,
            configJson["barVisibility"]["hemorrhage"],
            false,
            configJson["barVisibility"]["deathBlight"],
            false,
            configJson["barVisibility"]["frostbite"],
            false,
            configJson["barVisibility"]["sleep"],
            false,
            configJson["barVisibility"]["madness"],
            false,
            true,
            true,
            true,
            false
        };
    }

    return {
        configJson["components"]["hp"]["visible"],
        configJson["components"]["hp"]["hideText"],
        configJson["components"]["fp"]["visible"],
        configJson["components"]["fp"]["hideText"],
        configJson["components"]["stamina"]["visible"],
        configJson["components"]["stamina"]["hideText"],
        configJson["components"]["stagger"]["visible"],
        configJson["components"]["stagger"]["hideText"],
        configJson["components"]["poison"]["visible"],
        configJson["components"]["poison"]["hideText"],
        configJson["components"]["scarletRot"]["visible"],
        configJson["components"]["scarletRot"]["hideText"],
        configJson["components"]["hemorrhage"]["visible"],
        configJson["components"]["hemorrhage"]["hideText"],
        configJson["components"]["deathBlight"]["visible"],
        configJson["components"]["deathBlight"]["hideText"],
        configJson["components"]["frostbite"]["visible"],
        configJson["components"]["frostbite"]["hideText"],
        configJson["components"]["sleep"]["visible"],
        configJson["components"]["sleep"]["hideText"],
        configJson["components"]["madness"]["visible"],
        configJson["components"]["madness"]["hideText"],
        configJson["components"]["bestEffects"],
        configJson["components"]["immuneEffects"],
        configJson["components"]["dmgTypes"],
        configJson["components"]["neutralDmgTypes"]
    };
}

} // souls_vision