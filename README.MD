# Souls Vision
Souls Vision adds a customizable overlay to the top-right corner of the screen (by default), showing additional information about the currently targeted enemy, including HP, Stamina, Stance, and Buildup bars for effects (like Poison and Frostbite).

## Configuration
<details>
<summary>Config file details</summary>
The first time the game opens with the mod it will generate a default sv_config.json placing the bars at the top-right of the screen. Every time the config file is edited, it will be reloaded automatically.

### Fields
* **`debug`**: *bool* - Enables or disables debug mode. If set to `true`, a console window will open with the game, showing the same information as the one found in the `souls_vision.log`.
* **`bar`** | **`effectBar`**: *object* - Configuration for the bars, `bar` for the HP, FP, Stamina, Poise and `effectBar` for the Buildup bars.
    * **`position`**: *object* - Determines the bar’s on-screen position from top-left to bottom-right.
        * **`x`**: *int* - Horizontal position of the bar in pixels.
        * **`y`**: *int* - Vertical position of the bar in pixels.
    * **`size`**: *object* - Defines the bar’s dimensions.
        * **`width`**: *int* - Width of the bar in pixels.
        * **`height`**: *int* - Height of the bar in pixels.
    * **`hideText`**: *bool* - If set to `true`, hides the text displayed on the bar (e.g. “90/219”).
</details>

## Compatibility
Souls Vision was developed and tested on version 1.16, it is expected to work with previous and future versions as long as the game’s internal structures remain unchanged since it relies on memory reading. No incompatibilities with other mods are anticipated, but conflicts could arise if another mod hooks the same memory locations. Either way, let me know if any kind of error occurs.

## Installation
Add the `SoulsVision.dll` and `sv_assets` to the `mods` folder from [Elden Mod Loader](https://www.nexusmods.com/eldenring/mods/117)

## Changelog
* **[v0.1.1] - 24-11-2024**
    * Fixed the space between the Buildup Bars
* **[v0.1.0] - 24-11-2024**
    * First working version of the mod

## Credits  
Inspired by [Boss Checklist Overlay](https://www.nexusmods.com/eldenring/mods/3859): A lot of the code here helped me with the DirectX hooking.