//
// Created by PC-SAMUEL on 23/11/2024.
//

#include "config.h"
#include "globals.h"

namespace souls_vision {

BarSettings Config::statBarSettings;
BarSettings Config::effectBarSettings;
ImVec2 Config::effectBarIconSize = ImVec2(56, 48);
float Config::opacity;
bool Config::debug = false;

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

        if (!configJson.contains("opacity")) {
            configJson["opacity"] = 1.0f;
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

        if (!configJson.contains("effectBar")) {
            configJson["effectBar"] = {
                    {"position", {{"x", gGameWindowSize.width - 555 - 3}, {"y", 10}}},
                    {"size", {{"width", 555}, {"height", 40}}},
                    {"hideText", false}
            };
            updated = true;
        } else {
            if (!configJson["effectBar"].contains("position")) {
                configJson["effectBar"]["position"] = {{"x", gGameWindowSize.width - 555 - 3}, {"y", 10}};
                updated = true;
            }
            if (!configJson["effectBar"].contains("size")) {
                configJson["effectBar"]["size"] = {{"width", 555}, {"height", 40}};
                updated = true;
            }
            if (!configJson["effectBar"].contains("hideText")) {
                configJson["effectBar"]["hideText"] = false;
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

        Config::debug = configJson["debug"];
        opacity = configJson["opacity"];

        statBarSettings.position = ImVec2(configJson["statBar"]["position"]["x"], configJson["statBar"]["position"]["y"]);
        statBarSettings.size = ImVec2(configJson["statBar"]["size"]["width"], configJson["statBar"]["size"]["height"]);
        statBarSettings.hideText = configJson["statBar"]["hideText"];

        effectBarSettings.position = ImVec2(configJson["effectBar"]["position"]["x"], configJson["effectBar"]["position"]["y"]);
        effectBarSettings.size = ImVec2(configJson["effectBar"]["size"]["width"], configJson["effectBar"]["size"]["height"]);
        effectBarSettings.hideText = configJson["effectBar"]["hideText"];

        float iconWidth = effectBarSettings.size.y * 1.70f;
        effectBarIconSize = ImVec2(iconWidth, iconWidth * 0.85f);

    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::LoadConfig - Error: ") + e.what());
    }

    Logger::Info("Config loaded successfully.");
}

void Config::CreateConfig(const std::string &configFilePath) {
    Size barSize = {555, 40};

    try {
        nlohmann::json configJson;

        configJson["debug"] = false;

        configJson["opacity"] = 0.9f;

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