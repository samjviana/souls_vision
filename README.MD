# Souls Vision
Souls Vision adds a customizable overlay to the top-right corner of the screen (by default), showing additional information about the currently targeted enemy, including HP, Stamina, Stance, and Buildup bars for effects (like Poison and Frostbite).

## Configuration
<details>
<summary>Config file details</summary>
The first time the game opens with the mod it will generate a default sv_config.json placing the bars at the top-right of the screen. Every time the config file is edited, it will be reloaded automatically.

### Fields
* **`debug`**: *bool* - Enables or disables debug mode. If set to `true`, a console window will open with the game, showing the same information as the one found in the `souls_vision.log`.
* **`opacity`**: *float* - Opacity of the overlay, from 0.0 (fully transparent) to 1.0 (fully opaque). This option requires the game to be restarted to take effect.
* **`bestEffects`**: *int* - How many of the best effects to show on the overlay. The effects are sorted (left to right) by the lowest value necessary to trigger them. Default is `2`.
* **`statBar`** | **`effectBar`**: *object* - Configuration for the bars, `statBar` for the HP, FP, Stamina, Poise and `effectBar` for the Buildup bars.
    * **`position`**: *object* - Determines the bar’s on-screen position from top-left to bottom-right.
        * **`x`**: *int* - Horizontal position of the bar in pixels.
        * **`y`**: *int* - Vertical position of the bar in pixels.
    * **`size`**: *object* - Defines the bar’s dimensions.
        * **`width`**: *int* - Width of the bar in pixels.
        * **`height`**: *int* - Height of the bar in pixels.
    * **`hideText`**: *bool* - If set to `true`, hides the text displayed on the bar (e.g. “90/219”).
* **`barVisibility`**: *object* - Configuration for the visibility of the bars. All the bars are visible by default.
  * **`hp`**: *bool* - If set to `false`, the HP bar will be hidden.
  * **`fp`**: *bool* - If set to `false`, the FP bar will be hidden.
  * **`stamina`**: *bool* - If set to `false`, the Stamina bar will be hidden.
  * **`stagger`**: *bool* - If set to `false`, the Stagger bar will be hidden.
  * **`poison`**: *bool* - If set to `false`, the Poison bar will be hidden.
  * **`scarletRot`**: *bool* - If set to `false`, the Scarlet Rot bar will be hidden.
  * **`hemorrhage`**: *bool* - If set to `false`, the Hemorrhage bar will be hidden.
  * **`deathBlight`**: *bool* - If set to `false`, the Death Blight bar will be hidden.
  * **`frostbite`**: *bool* - If set to `false`, the Frostbite bar will be hidden.
  * **`sleep`**: *bool* - If set to `false`, the Sleep bar will be hidden.
  * **`madness`**: *bool* - If set to `false`, the Madness bar will be hidden.
</details>

## Compatibility
Souls Vision was developed and tested on version 1.16, it is expected to work with previous and future versions as long as the game’s internal structures remain unchanged since it relies on memory reading. No incompatibilities with other mods are anticipated, but conflicts could arise if another mod hooks the same memory locations. Either way, let me know if any kind of error occurs.

## Installation
Add the `SoulsVision.dll` and `sv_assets` to the `mods` folder from [Elden Mod Loader](https://www.nexusmods.com/eldenring/mods/117)

## Changelog
* **[v0.1.5] - 25-11-2024**
  * Added icons to show best effects to use against the targeted enemy
* **[v0.1.4] - 25-11-2024**
  * Added custom visibility config to all the bars
* **[v0.1.3] - 25-11-2024**
  * Fixed the position of the bars that were overlapping as their size increased
* **[v0.1.2] - 24-11-2024**
  * Allow configurable opacity to the overlay
* **[v0.1.1] - 24-11-2024**
  * Fixed the space between the Buildup Bars
* **[v0.1.0] - 24-11-2024**
  * First working version of the mod

## Credits  
[Boss Checklist Overlay](https://www.nexusmods.com/eldenring/mods/3859): A lot of the code here helped me with the DirectX hooking.</br>
[Posture Bar](https://www.nexusmods.com/eldenring/mods/3405): The concept of showing additional "bars" gave me the initial idea.</br>
[Hunter Pie](https://www.nexusmods.com/monsterhunterrise/mods/181): This is first contact that I had with this idea of Overlay. If you're a Monster Hunter player, I highly recommend it.</br>