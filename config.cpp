//
// Created by PC-SAMUEL on 23/11/2024.
//

#include "config.h"
#include "globals.h"

namespace souls_vision {

BarSettings Config::defaultBarSettings;
BarSettings Config::effectBarSettings;
float Config::opacity;
bool Config::debug = false;

void Config::LoadConfig(const std::string& configFilePath)  {
    try {
        // Check if the file exists
        if (!std::filesystem::exists(configFilePath)) {
            Logger::Info("Config file not found. Creating a new one...");
            CreateConfig(configFilePath);
        }

        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open sv_config.json");
        }

        nlohmann::json configJson;
        configFile >> configJson;

        Config::debug = configJson["debug"];

        opacity = configJson["opacity"];

        defaultBarSettings.position = ImVec2(configJson["bar"]["position"]["x"], configJson["bar"]["position"]["y"]);
        defaultBarSettings.size = ImVec2(configJson["bar"]["size"]["width"], configJson["bar"]["size"]["height"]);
        defaultBarSettings.currentValue = configJson["bar"]["current"];
        defaultBarSettings.maxValue = configJson["bar"]["max"];
        defaultBarSettings.hideText = configJson["bar"]["hideText"];

        effectBarSettings.position = ImVec2(configJson["effectBar"]["position"]["x"], configJson["bar"]["position"]["y"]);
        effectBarSettings.size = ImVec2(configJson["effectBar"]["size"]["width"], configJson["bar"]["size"]["height"]);
        effectBarSettings.currentValue = configJson["effectBar"]["current"];
        effectBarSettings.maxValue = configJson["effectBar"]["max"];
        effectBarSettings.hideText = configJson["effectBar"]["hideText"];
    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::LoadConfig - Error: ") + e.what());
    }

    Logger::Info("Config loaded");
}

void Config::CreateConfig(const std::string &configFilePath) {
    Size barSize = {555, 40};

    try {
        nlohmann::json configJson;

        configJson["debug"] = false;

        configJson["opacity"] = 1.0f;

        configJson["bar"]["position"]["x"] = gGameWindowSize.width - barSize.width - 5;
        configJson["bar"]["position"]["y"] = 10;
        configJson["bar"]["size"]["width"] = barSize.width;
        configJson["bar"]["size"]["height"] = barSize.height;
        configJson["bar"]["hideText"] = false;

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