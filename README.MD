# Souls Vision
Souls Vision adds a customizable overlay to the top-right corner of the screen (by default), showing additional information about the currently targeted enemy, including HP, Stamina, Stance, and Buildup bars for effects (like Poison and Frostbite).

## Features
* **Stat Bars**: Shows the current and maximum HP, FP, Stamina, and Stagger/Poise of the targeted enemy.
  * The FP bar usually only appear to "player-like" enemies
* **Best Effects**: Shows the best effects to use against the targeted enemy, based on the lowest value necessary to trigger them.
* **Immune Effects**: Shows the effects that the targeted enemy is immune to in gray at the right of the best effects.
* **Damage Types**: Shows the damage types that the targeted enemy is weak to, resistant to, or neutral to with icons highlighting how good or bad they are.
  * The neutral damage types are hidden by default, but can be shown by changing the config file.
* **Buildup Bars**: Shows the current buildup of effects like Poison, Scarlet Rot, Hemorrhage, Death Blight, Frostbite, Sleep, and Madness.
  * Only appear if the enemy has some effect being applied to it.
* **Customizable**: Highly customizable, with options to change the positions, sizes, and visibility of the bars.
  * The config file is generated the first time the game is opened with the mod, and it is reloaded automatically every time it is edited.
* **Draggable Overlay**: The overlay can be dragged around the screen by clicking and dragging it.
  * The parameter `dragOverlay` must be set to `true` in the config file to enable this feature.
  * I recommend enabling entering a menu (like Equipment) to show mouse cursor to make it easier to drag the overlay. (I'm working on a better way to do this)

## Configuration
<details>
<summary>Config file details</summary>
The first time the game opens with the mod it will generate a default sv_config.json placing the bars at the top-right of the screen. Every time the config file is edited, it will be reloaded automatically.

### Fields
* **`dragOverlay`**: *bool* - If set to `true`, the overlay can be dragged around the screen by clicking and dragging it.
* **`debug`**: *bool* - Enables or disables debug mode. If set to `true`, a console window will open with the game, showing the same information as the one found in the `souls_vision.log`.
* **`opacity`**: *float* - Opacity of the overlay, from 0.0 (fully transparent) to 1.0 (fully opaque). This option requires the game to be restarted to take effect.
* **`bestEffects`**: *int* - How many of the best effects to show on the overlay. The effects are sorted (left to right) by the lowest value necessary to trigger them. Default is `2`.
* **`bestEffectIconSize`**: *int* - Size of the best effect icons. Default is `33`.
* **`dmgTypeIconSize`**: *int* - Size of the damage type icons. Default is `30`.
* **`fontSize`**: *float* - Font size of the text displayed on the bars. Default is `18.0`.
* **`statBarSpacing`**: *int* - Spacing between the bars, the space used around the "bestEffect" will be half of this. Default is `0`.
* **`statBar`**: *object* - Configuration related to the bars.
    * **`position`**: *object* - Determines the bar’s on-screen position from top-left to bottom-right.
        * **`x`**: *int* - Horizontal position of the bar in pixels.
        * **`y`**: *int* - Vertical position of the bar in pixels.
    * **`size`**: *object* - Defines the bar’s dimensions.
        * **`width`**: *int* - Width of the bar in pixels.
        * **`height`**: *int* - Height of the bar in pixels.
    * **`hideText`**: *bool* - If set to `true`, hides the text displayed on the bar (e.g. “90/219”).
* **`componentVisibility`**: *object* - Configuration for the visibility of the bars. All the bars are visible by default.
  * **`hp`**: *bool* - If set to `false`, the HP bar will be hidden. Default is `true`.
  * **`fp`**: *bool* - If set to `false`, the FP bar will be hidden. Default is `true`.
  * **`stamina`**: *bool* - If set to `false`, the Stamina bar will be hidden. Default is `true`.
  * **`stagger`**: *bool* - If set to `false`, the Stagger bar will be hidden. Default is `true`.
  * **`poison`**: *bool* - If set to `false`, the Poison bar will be hidden. Default is `true`.
  * **`scarletRot`**: *bool* - If set to `false`, the Scarlet Rot bar will be hidden. Default is `true`.
  * **`hemorrhage`**: *bool* - If set to `false`, the Hemorrhage bar will be hidden. Default is `true`.
  * **`deathBlight`**: *bool* - If set to `false`, the Death Blight bar will be hidden. Default is `true`.
  * **`frostbite`**: *bool* - If set to `false`, the Frostbite bar will be hidden. Default is `true`.
  * **`sleep`**: *bool* - If set to `false`, the Sleep bar will be hidden. Default is `true`.
  * **`madness`**: *bool* - If set to `false`, the Madness bar will be hidden. Default is `true`.
  * **`bestEffects`**: *bool* - If set to `false`, the Best Effects against the enemy will be hidden. Default is `true`.
  * **`immuneEffects`**: *bool* - If set to `false`, the effects that the enemy is immune to will be hidden. Default is `true`.
  * **`dmgTypes`**: *bool* - If set to `false`, the Damage Type information will be hidden. Default is `true`.
  * **`neutralDmgTypes`**: *bool* - If set to `true`, the Neutral Damage Type information will be shown. Default is `false`.
</details>

## Compatibility
Souls Vision was developed and tested on version 1.16, it is expected to work with previous and future versions as long as the game’s internal structures remain unchanged since it relies on memory reading. No incompatibilities with other mods are anticipated, but conflicts could arise if another mod hooks the same memory locations. Either way, let me know if any kind of error occurs.

## Installation
Add the `SoulsVision.dll` and `sv_assets` to the `mods` folder from [Elden Mod Loader](https://www.nexusmods.com/eldenring/mods/117)

## Changelog
* **[v0.3.0] - 07-12-2024**
  * Added the Weak, Neutral and Resistant damage types to the overlay
  * Added more customization to the overlay components
    * It's possible to control the visibility of more components individually
    * Added a config parameter to control the font size of the overlay
    * The `opacity` parameter no longer requires the game to be restarted to take effect
* **[v0.2.2] - 28-11-2024**
  * Changed the overlay to use ER font
* **[v0.2.1] - 28-11-2024**
  * Added a way to load textures in grayscale
  * Added the effects that the enemy are immune to the overlay
  * Tweaked the size of the icons and the opacity of the bars bg
* **[v0.2.0] - 27-11-2024**
  * Added a config parameter to customize the vertical space between the bars
  * Added a config parameter to customize the size of the "best effect" icon
  * Removed the condition that prevented the stamina bar from rendering
  * Completely reworked the way the bars are rendered to be tied to a `ImGui` window
    * This change allows the overlay to be dragged around the screen
    * This "drag" mode can be toggled on the config file
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