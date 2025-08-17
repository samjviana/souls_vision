//
// Created by PC-SAMUEL on 23/11/2024.
//

#include "config.h"
#include "globals.h"
#include "json.hpp"

#include <regex>
#include <toml.hpp>

namespace souls_vision {

Components Config::components;
BarSettings Config::statBarSettings;
float Config::bestEffectIconSize = 39;
float Config::dmgTypeIconSize = 30;
float Config::effectBarIconSize = 56;
int Config::bestEffects = 3;
int Config::statBarSpacing = 0;
float Config::fontSize = 18.0f;
float Config::opacity;
int Config::delay = 0;
bool Config::debug = false;
bool Config::dragOverlay = false;
bool Config::configUpdated = false;
bool Config::opacityUpdated = false;
bool Config::fontSizeUpdated = false;
bool Config::hideBlightMadness = false;
int Config::maxEffectBars = 7;

void Config::SaveConfig(const std::string& configFilePath) {
    try {
        toml::table configToml;

        configToml.insert_or_assign("general", toml::table{
                {"debug", debug},
                {"dragOverlay", dragOverlay},
                {"fontSize", fontSize},
                {"opacity", opacity},
                {"delay", delay},
        });

        configToml.insert_or_assign("appearance", toml::table{
                {"bestEffects", bestEffects},
                {"maxEffectBars", maxEffectBars},
                {"bestEffectIconSize", bestEffectIconSize},
                {"dmgTypeIconSize", effectBarIconSize},
                {"statBarIconSize", statBarSpacing},
                {"statBarSpacing", statBarSpacing},
                {"hideBlightMadness", hideBlightMadness}
        });

        toml::table statBar;
        statBar.insert_or_assign("position", toml::table{
                {"x", statBarSettings.position.x},
                {"y", statBarSettings.position.y}
        });
        statBar.insert_or_assign("size", toml::table{
                {"width", statBarSettings.size.x},
                {"height", statBarSettings.size.y}
        });
        configToml.insert_or_assign("statBar", statBar);

        toml::table componentsTable;
        componentsTable.insert_or_assign("hp", toml::table{{"visible", components.hp.visible}, {"hideText", components.hp.hideText}});
        componentsTable.insert_or_assign("fp", toml::table{{"visible", components.fp.visible}, {"hideText", components.fp.hideText}});
        componentsTable.insert_or_assign("stamina", toml::table{{"visible", components.stamina.visible}, {"hideText", components.stamina.hideText}});
        componentsTable.insert_or_assign("stagger", toml::table{{"visible", components.stagger.visible}, {"hideText", components.stagger.hideText}});
        componentsTable.insert_or_assign("poison", toml::table{{"visible", components.poison.visible}, {"hideText", components.poison.hideText}});
        componentsTable.insert_or_assign("scarletRot", toml::table{{"visible", components.scarletRot.visible}, {"hideText", components.scarletRot.hideText}});
        componentsTable.insert_or_assign("hemorrhage", toml::table{{"visible", components.hemorrhage.visible}, {"hideText", components.hemorrhage.hideText}});
        componentsTable.insert_or_assign("deathBlight", toml::table{{"visible", components.deathBlight.visible}, {"hideText", components.deathBlight.hideText}});
        componentsTable.insert_or_assign("frostbite", toml::table{{"visible", components.frostbite.visible}, {"hideText", components.frostbite.hideText}});
        componentsTable.insert_or_assign("sleep", toml::table{{"visible", components.sleep.visible}, {"hideText", components.sleep.hideText}});
        componentsTable.insert_or_assign("madness", toml::table{{"visible", components.madness.visible}, {"hideText", components.madness.hideText}});
        componentsTable.insert_or_assign("bestEffects", components.bestEffects);
        componentsTable.insert_or_assign("immuneEffects", components.immuneEffects);
        componentsTable.insert_or_assign("dmgTypes", components.dmgTypes);
        componentsTable.insert_or_assign("neutralDmgTypes", components.neutralDmgTypes);
        configToml.insert_or_assign("components", componentsTable);

        std::ofstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open " + configFilePath + " for writing.");
        }

        configFile << configToml;
        configFile.close();

        AddComments(configFilePath);
    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::SaveConfig - Error: ") + e.what());
    }
}

void Config::LoadConfig(const std::string& configFilePath) {
    try {
        toml::table configToml;

        if (!std::filesystem::exists(configFilePath)) {
            Logger::Info("Config file not found. Creating a new one...");
            CreateConfig(configFilePath);

            std::ifstream configFile(configFilePath);
            if (!configFile.is_open()) {
                throw std::runtime_error("Failed to open " + configFilePath + " after creation.");
            }
            configToml = toml::parse(configFile);
        } else {
            std::ifstream configFile(configFilePath);
            if (!configFile.is_open()) {
                throw std::runtime_error("Failed to open " + configFilePath);
            }
            configToml = toml::parse(configFile);
        }

        debug = configToml["general"]["debug"].value_or(false);
        dragOverlay = configToml["general"]["dragOverlay"].value_or(false);
        hideBlightMadness = configToml["general"]["hideBlightMadness"].value_or(false);
        fontSize = configToml["general"]["fontSize"].value_or(18.0f);
        opacity = configToml["general"]["opacity"].value_or(0.9f);
        delay = configToml["general"]["delay"].value_or(0);

        fontSizeUpdated = (fontSize != configToml["general"]["fontSize"].value_or(fontSize));
        opacityUpdated = (opacity != configToml["general"]["opacity"].value_or(opacity));

        bestEffects = configToml["appearance"]["bestEffects"].value_or(3);
        bestEffectIconSize = configToml["appearance"]["bestEffectIconSize"].value_or(39);
        bestEffectIconSize = bestEffectIconSize * 0.85f;
        dmgTypeIconSize = configToml["appearance"]["dmgTypeIconSize"].value_or(30);
        dmgTypeIconSize = dmgTypeIconSize;
        statBarSpacing = configToml["appearance"]["statBarSpacing"].value_or(0);
        maxEffectBars = configToml["appearance"]["maxEffectBars"].value_or(7);

        auto statBarTable = configToml["statBar"].as_table();
        if (statBarTable) {
            auto positionTable = statBarTable->at("position").as_table();
            if (positionTable) {
                statBarSettings.position.x = positionTable->at("x").value_or(gGameWindowSize.width - 555 - 5);
                statBarSettings.position.y = positionTable->at("y").value_or(10);
            }

            auto sizeTable = statBarTable->at("size").as_table();
            if (sizeTable) {
                statBarSettings.size.x = sizeTable->at("width").value_or(555);
                statBarSettings.size.y = sizeTable->at("height").value_or(40);
            }
        }

        float iconWidth = statBarSettings.size.y * 1.70f;
        effectBarIconSize = iconWidth;

        auto componentsTable = configToml["components"].as_table();
        if (componentsTable) {
            components.hp.visible = componentsTable->at("hp").as_table()->at("visible").value_or(true);
            components.hp.hideText = componentsTable->at("hp").as_table()->at("hideText").value_or(false);

            components.fp.visible = componentsTable->at("fp").as_table()->at("visible").value_or(true);
            components.fp.hideText = componentsTable->at("fp").as_table()->at("hideText").value_or(false);

            components.stamina.visible = componentsTable->at("stamina").as_table()->at("visible").value_or(true);
            components.stamina.hideText = componentsTable->at("stamina").as_table()->at("hideText").value_or(false);

            components.stagger.visible = componentsTable->at("stagger").as_table()->at("visible").value_or(true);
            components.stagger.hideText = componentsTable->at("stagger").as_table()->at("hideText").value_or(false);

            components.poison.visible = componentsTable->at("poison").as_table()->at("visible").value_or(true);
            components.poison.hideText = componentsTable->at("poison").as_table()->at("hideText").value_or(false);

            components.scarletRot.visible = componentsTable->at("scarletRot").as_table()->at("visible").value_or(true);
            components.scarletRot.hideText = componentsTable->at("scarletRot").as_table()->at("hideText").value_or(false);

            components.hemorrhage.visible = componentsTable->at("hemorrhage").as_table()->at("visible").value_or(true);
            components.hemorrhage.hideText = componentsTable->at("hemorrhage").as_table()->at("hideText").value_or(false);

            components.deathBlight.visible = componentsTable->at("deathBlight").as_table()->at("visible").value_or(true);
            components.deathBlight.hideText = componentsTable->at("deathBlight").as_table()->at("hideText").value_or(false);

            components.frostbite.visible = componentsTable->at("frostbite").as_table()->at("visible").value_or(true);
            components.frostbite.hideText = componentsTable->at("frostbite").as_table()->at("hideText").value_or(false);

            components.sleep.visible = componentsTable->at("sleep").as_table()->at("visible").value_or(true);
            components.sleep.hideText = componentsTable->at("sleep").as_table()->at("hideText").value_or(false);

            components.madness.visible = componentsTable->at("madness").as_table()->at("visible").value_or(true);
            components.madness.hideText = componentsTable->at("madness").as_table()->at("hideText").value_or(false);

            components.bestEffects = componentsTable->at("bestEffects").value_or(true);
            components.immuneEffects = componentsTable->at("immuneEffects").value_or(true);
            components.dmgTypes = componentsTable->at("dmgTypes").value_or(true);
            components.neutralDmgTypes = componentsTable->at("neutralDmgTypes").value_or(false);
        }

        Logger::Info("Config loaded successfully.");
    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::LoadConfig - Error: ") + e.what());
    }
}

void Config::CreateConfig(const std::string &configFilePath) {
    Size barSize = {600, 40};

    try {
        toml::table configToml;

        configToml.insert_or_assign("general", toml::table{
                {"debug", false},
                {"dragOverlay", false},
                {"hideBlightMadness", false},
                {"fontSize", 18.0f},
                {"opacity", 0.9f},
                {"delay", 0}
        });

        configToml.insert_or_assign("appearance", toml::table{
                {"bestEffects", 3},
                {"bestEffectIconSize", 39},
                {"dmgTypeIconSize", 30},
                {"statBarSpacing", 0},
                {"maxEffectBars", 7}
        });

        toml::table statBar;
        statBar.insert_or_assign("position", toml::table{
                {"x", gGameWindowSize.width - barSize.width - 5},
                {"y", 10}
        });
        statBar.insert_or_assign("size", toml::table{
                {"width", barSize.width},
                {"height", barSize.height}
        });
        configToml.insert_or_assign("statBar", statBar);

        toml::table componentsToml;
        componentsToml.insert_or_assign("hp", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("fp", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("stamina", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("stagger", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("poison", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("scarletRot", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("hemorrhage", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("deathBlight", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("frostbite", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("sleep", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("madness", toml::table{{"visible", true}, {"hideText", false}});
        componentsToml.insert_or_assign("bestEffects", true);
        componentsToml.insert_or_assign("immuneEffects", true);
        componentsToml.insert_or_assign("dmgTypes", true);
        componentsToml.insert_or_assign("neutralDmgTypes", false);
        configToml.insert_or_assign("components", componentsToml);

        std::ofstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to create sv_config.toml");
        }

        configFile << configToml;
        configFile.close();

        AddComments(configFilePath);
    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::CreateConfig - Error: ") + e.what());
    }
}

void Config::AddComments(const std::string &configFilePath) {
    try {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Failed to open " + configFilePath);
        }

        std::ostringstream buffer;
        buffer << configFile.rdbuf();
        std::string content = buffer.str();
        configFile.close();

        std::map<std::string, std::string> comments = {
                // general
                {"debug", "Enables or disables debug mode. If set to `true`, a console window will open with the game, showing the same information as the one found in the `souls_vision.log`"},
                {"dragOverlay", "If set to `true`, the overlay can be dragged around the screen by clicking and dragging it"},
                {"fontSize", "Font size of the text displayed on the bars. Default is `18.0`"},
                {"opacity", "Opacity of the overlay, from 0.0 (fully transparent) to 1.0 (fully opaque). This option requires the game to be restarted to take effect"},
                {"delay", "Delay in milliseconds before initializing the overlay. Default is `0`"},
                // appearance
                {"bestEffectIconSize", "Size of the best effect icons. Default is `33`"},
                {"bestEffects", "How many of the best effects to show on the overlay. The effects are sorted (left to right) by the lowest value necessary to trigger them. Default is `2`"},
                {"dmgTypeIconSize", "Size of the damage type icons. Default is 30."},
                {"hideBlightMadness", "If true, hides Death Blight and Madness bars for common enemies. Default is false."},
                {"maxEffectBars", "Maximum number of effect bars to show. Default is 7."},
                {"statBarSpacing", "Spacing between the bars. Default is 0."},
                // statBar.position
                {"x", "Horizontal position of the bar in pixels"},
                {"y", "Vertical position of the bar in pixels"},
                // statBar.size
                {"width", "Width of the bar in pixels"},
                {"height", "Height of the bar in pixels"},
                // components
                {"hp", "HP Bar configuration"},
                {"fp", "FP Bar configuration"},
                {"stamina", "Stamina Bar configuration"},
                {"stagger", "Stagger Bar configuration"},
                {"poison", "Poison Bar configuration"},
                {"scarletRot", "Scarlet Rot Bar configuration"},
                {"hemorrhage", "Hemorrhage Bar configuration"},
                {"deathBlight", "Death Blight Bar configuration"},
                {"frostbite", "Frostbite Bar configuration"},
                {"sleep", "Sleep Bar configuration"},
                {"madness", "Madness Bar configuration"},
                {"bestEffects", "If set to `false`, the Best Effects against the enemy will be hidden. Default is `true`"},
                {"immuneEffects", "If set to `false`, the effects that the enemy is immune to will be hidden. Default is `true`"},
                {"dmgTypes", "If set to `false`, the Damage Type information will be hidden. Default is `true`"},
                {"neutralDmgTypes", "If set to `true`, the Neutral Damage Type information will be shown. Default is `false`"},
                // components.bar
                {"visible", "If set to `false`, the bar will be hidden. Default is `true`"},
                {"hideText", "If set to `true`, hides the text displayed on the bar (e.g. “90/219”). Default is `false`"}
        };

        std::regex configRegex(R"((\w+)\s*=\s*.+)");
        std::string updatedContent = content;

        for (const auto& [key, comment] : comments) {
            std::regex keyRegex(key + R"(\s*=\s*.+)");
            updatedContent = std::regex_replace(updatedContent, keyRegex, "$&  # " + comment);
        }

        std::ofstream outFile(configFilePath);
        if (!outFile.is_open()) {
            throw std::runtime_error("Failed to write to " + configFilePath);
        }

        outFile << updatedContent;
        outFile.close();

        Logger::Info("Comments added to config file successfully.");
    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::AddComments - Error: ") + e.what());
    }
}

} // souls_vision