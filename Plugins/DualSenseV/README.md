# Unreal-DualSense5-Plugin
An Unreal Engine plugin bringing an almost complete support for the DualSense5 controller. 

## Features
+ Control over DualSense's specific features such as trigger resistances
+ Support with Unreal built-in force feedback system

## TODO
+ Support local multiplayer with multiple controllers
+ Connect all Gamepad buttons and axes events
+ Connect all DualSense buttons and axes events
+ Handle controller reconnection
+ CommonUI support for Playstation icons
+ Debug visualizer for all buttons and axes
+ Add timer when setting rumbles
+ Haptic Feedback via Audio files

+ Find out why IA_Shoot action doesn't give a proper trigger axis value

### Haptic Feedback via Audio files
It was achieved in the [Wujek-Dualsense-API](https://github.com/WujekFoliarz/Wujek-Dualsense-API) nuget package in C#. 

It used [NAudio](https://github.com/naudio/NAudio) to connect to the DualSense audio devices and send wave audio channels:
- Find the DualSense's audio device by enumerating through all connected audio devices on the machine
(since it is independent of the actual gamepad, you need to be prudent to find the matching audio device).
- Prepare working audio channels and create a connection (it used Wasapi) to the audio device.
- Read audio files (it used WAV files in 48KHz 32-bit float) and send samples to the working audio channels
so it can send them back to the DualSense's audio device.

DualSense audio device is quadraphonic, meaning it uses 4 audio channels:
- Channel 0 is the left-side channel for the headphone when plugged
- Channel 1 is the mono speaker and the right-side channel for the headphone when plugged
- Channels 2 and 3 are for the haptics, respectively left and right sides

You can use them individually or all at the same time to achieve different effects.

## Resources
+ [Nielk1's TriggerEffectGenerator.cs](https://gist.github.com/Nielk1/6d54cc2c00d2201ccb8c2720ad7538db)
+ [Nielk1's notes on DualSense firmware internal](https://controllers.fandom.com/wiki/Sony_DualSense)
+ [Wujek's C# DualSense API](https://github.com/WujekFoliarz/Wujek-Dualsense-API)
+ [Wujek's Dying Light 1 DualSense mod](https://github.com/WujekFoliarz/Dying-Light-1-DUALSENSE-MOD)