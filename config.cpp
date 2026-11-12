//
// Created by PC-SAMUEL on 23/11/2024.
//

#include "config.h"
#include "globals.h"

#include <regex>
#include <cmath>
#include <toml.hpp>

namespace souls_vision {

namespace {

// Checks whether the string is valid UTF-8 (required by the TOML parser).
bool IsValidUtf8(const std::string& text) {
    size_t i = 0;
    while (i < text.size()) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        size_t extra = 0;
        if (c < 0x80) {
            extra = 0;
        } else if ((c & 0xE0) == 0xC0) {
            extra = 1;
        } else if ((c & 0xF0) == 0xE0) {
            extra = 2;
        } else if ((c & 0xF8) == 0xF0) {
            extra = 3;
        } else {
            return false;
        }
        if (i + extra >= text.size()) {
            return false;
        }
        for (size_t j = 1; j <= extra; j++) {
            if ((static_cast<unsigned char>(text[i + j]) & 0xC0) != 0x80) {
                return false;
            }
        }
        i += extra + 1;
    }
    return true;
}

// Reads the config file as UTF-8. Comments written in a legacy ANSI code page
// (e.g. GBK with Chinese comments) would otherwise make the TOML parser throw
// and silently reset every setting, so such files are transcoded from the
// system code page first.
std::string ReadFileAsUtf8(const std::string& path) {
    std::ifstream configFile(path, std::ios::binary);
    if (!configFile.is_open()) {
        throw std::runtime_error("Failed to open " + path);
    }
    std::string raw((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());
    if (raw.empty() || IsValidUtf8(raw)) {
        return raw;
    }

    const int wideLength = MultiByteToWideChar(CP_ACP, 0, raw.c_str(), static_cast<int>(raw.size()), nullptr, 0);
    if (wideLength <= 0) {
        return raw;
    }
    std::wstring wide(wideLength, L'\0');
    MultiByteToWideChar(CP_ACP, 0, raw.c_str(), static_cast<int>(raw.size()), wide.data(), wideLength);

    const int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), wideLength, nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0) {
        return raw;
    }
    std::string utf8(utf8Length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), wideLength, utf8.data(), utf8Length, nullptr, nullptr);
    return utf8;
}

// Safe table lookups: a missing key or a missing sub-table must never abort the whole config load.
template <typename Key>
const toml::table* GetTable(const toml::table& table, const Key& key) {
    return table[key].as_table();
}

// Number readers accepting both integers and floats (e.g. `x = 1955` and `x = 1955.0`).
template <typename NodeView>
int GetInt(const NodeView& node, int fallback) {
    if (auto value = node.template value<int64_t>()) {
        return static_cast<int>(*value);
    }
    if (auto value = node.template value<double>()) {
        return static_cast<int>(std::lround(*value));
    }
    return fallback;
}

template <typename NodeView>
float GetFloat(const NodeView& node, float fallback) {
    if (auto value = node.template value<double>()) {
        return static_cast<float>(*value);
    }
    if (auto value = node.template value<int64_t>()) {
        return static_cast<float>(*value);
    }
    return fallback;
}

} // namespace

Components Config::components;
BarSettings Config::statBarSettings;
ImVec2 Config::bestEffectIconSize = ImVec2(39, 33);
ImVec2 Config::dmgTypeIconSize = ImVec2(30, 30);
ImVec2 Config::effectBarIconSize = ImVec2(56, 48);
int Config::bestEffects = 7;
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
std::string Config::assetPack = "default";

void Config::SaveConfig(const std::string& configFilePath) {
    try {
        toml::table configToml;

        configToml.insert_or_assign("general", toml::table{
                {"debug", debug},
                {"dragOverlay", dragOverlay},
                {"fontSize", fontSize},
                {"opacity", opacity},
                {"delay", delay},
                {"assetPack", assetPack},
        });

        configToml.insert_or_assign("appearance", toml::table{
                {"bestEffects", bestEffects},
                {"maxEffectBars", maxEffectBars},
                {"bestEffectIconSize", bestEffectIconSize.x},
                {"dmgTypeIconSize", dmgTypeIconSize.x},
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

bool Config::LoadConfig(const std::string& configFilePath) {
    try {
        toml::table configToml;

        if (!std::filesystem::exists(configFilePath)) {
            Logger::Info("Config file not found. Creating a new one...");
            CreateConfig(configFilePath);

            configToml = toml::parse(ReadFileAsUtf8(configFilePath));
        } else {
            configToml = toml::parse(ReadFileAsUtf8(configFilePath));
        }

        debug = configToml["general"]["debug"].value_or(false);
        dragOverlay = configToml["general"]["dragOverlay"].value_or(false);
        hideBlightMadness = configToml["general"]["hideBlightMadness"].value_or(false);
        fontSize = GetFloat(configToml["general"]["fontSize"], 18.0f);
        opacity = GetFloat(configToml["general"]["opacity"], 0.9f);
        delay = GetInt(configToml["general"]["delay"], 0);
        assetPack = configToml["general"]["assetPack"].value_or(std::string("default"));

        fontSizeUpdated = (fontSize != GetFloat(configToml["general"]["fontSize"], fontSize));
        opacityUpdated = (opacity != GetFloat(configToml["general"]["opacity"], opacity));

        bestEffects = GetInt(configToml["appearance"]["bestEffects"], 7);
        bestEffectIconSize.x = GetInt(configToml["appearance"]["bestEffectIconSize"], 39);
        bestEffectIconSize.y = bestEffectIconSize.x * 0.85f;
        dmgTypeIconSize.x = GetInt(configToml["appearance"]["dmgTypeIconSize"], 30);
        dmgTypeIconSize.y = dmgTypeIconSize.x;
        statBarSpacing = GetInt(configToml["appearance"]["statBarSpacing"], 0);
        maxEffectBars = GetInt(configToml["appearance"]["maxEffectBars"], 7);

        if (const auto statBarTable = GetTable(configToml, "statBar")) {
            if (const auto positionTable = GetTable(*statBarTable, "position")) {
                statBarSettings.position.x = GetInt((*positionTable)["x"], gGameWindowSize.width - 555 - 5);
                statBarSettings.position.y = GetInt((*positionTable)["y"], 10);
            }

            if (const auto sizeTable = GetTable(*statBarTable, "size")) {
                statBarSettings.size.x = GetInt((*sizeTable)["width"], 555);
                statBarSettings.size.y = GetInt((*sizeTable)["height"], 40);
            }
        }

        // An invalid (zero/negative) size would make the whole overlay invisible.
        if (statBarSettings.size.x <= 0) {
            statBarSettings.size.x = 555;
        }
        if (statBarSettings.size.y <= 0) {
            statBarSettings.size.y = 40;
        }

        float iconWidth = statBarSettings.size.y * 1.70f;
        effectBarIconSize = ImVec2(iconWidth, iconWidth * 0.85f);

        if (const auto componentsTable = GetTable(configToml, "components")) {
            // Reads `<components>.<name>.<key>`. A missing section or key falls back to the default
            // instead of throwing, so a partially written config still loads.
            const auto readComponent = [componentsTable](const char* name, const char* key, bool fallback) {
                const auto componentTable = GetTable(*componentsTable, name);
                return componentTable ? (*componentTable)[key].value_or(fallback) : fallback;
            };

            components.hp.visible = readComponent("hp", "visible", true);
            components.hp.hideText = readComponent("hp", "hideText", false);

            components.fp.visible = readComponent("fp", "visible", true);
            components.fp.hideText = readComponent("fp", "hideText", false);

            components.stamina.visible = readComponent("stamina", "visible", true);
            components.stamina.hideText = readComponent("stamina", "hideText", false);

            components.stagger.visible = readComponent("stagger", "visible", true);
            components.stagger.hideText = readComponent("stagger", "hideText", false);

            components.poison.visible = readComponent("poison", "visible", true);
            components.poison.hideText = readComponent("poison", "hideText", false);

            components.scarletRot.visible = readComponent("scarletRot", "visible", true);
            components.scarletRot.hideText = readComponent("scarletRot", "hideText", false);

            components.hemorrhage.visible = readComponent("hemorrhage", "visible", true);
            components.hemorrhage.hideText = readComponent("hemorrhage", "hideText", false);

            components.deathBlight.visible = readComponent("deathBlight", "visible", true);
            components.deathBlight.hideText = readComponent("deathBlight", "hideText", false);

            components.frostbite.visible = readComponent("frostbite", "visible", true);
            components.frostbite.hideText = readComponent("frostbite", "hideText", false);

            components.sleep.visible = readComponent("sleep", "visible", true);
            components.sleep.hideText = readComponent("sleep", "hideText", false);

            components.madness.visible = readComponent("madness", "visible", true);
            components.madness.hideText = readComponent("madness", "hideText", false);

            components.bestEffects = (*componentsTable)["bestEffects"].value_or(true);
            components.immuneEffects = (*componentsTable)["immuneEffects"].value_or(true);
            components.dmgTypes = (*componentsTable)["dmgTypes"].value_or(true);
            components.neutralDmgTypes = (*componentsTable)["neutralDmgTypes"].value_or(false);
        }

        Logger::Info("Config loaded successfully.");
        return true;
    } catch (const std::exception& e) {
        Logger::Error(std::string("Config::LoadConfig - Error: ") + e.what());
        Logger::Error("Keeping the current config file untouched. Check its syntax (values must be UTF-8, numbers may be written with or without decimals).");
        return false;
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
                {"delay", 0},
                {"assetPack", "default"}
        });

        configToml.insert_or_assign("appearance", toml::table{
                {"bestEffects", 7},
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
                {"assetPack", "Name of the subfolder inside `sv_assets` to load icon textures from. Missing files fall back to the textures bundled in the DLL. Set to `default` to use only the bundled textures. Changing this requires restarting the game. Default is `default`"},
                // appearance
                {"bestEffectIconSize", "Size of the best effect icons. Default is `33`"},
                {"bestEffects", "How many of the best effects to show on the overlay. The effects are sorted (left to right) by the lowest value necessary to trigger them. Default is `7`"},
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