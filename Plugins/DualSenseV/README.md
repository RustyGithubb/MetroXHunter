# Unreal-DualSense5-Plugin
An Unreal Engine plugin bringing an almost complete support for the DualSense5 controller. 

## Features
+ Integrated with Unreal's input system and force feedback system
+ Adaptative triggers with an integrated library of effects
+ Custom DualSense input events for all gamepad buttons
+ Support for gamepad auto-reconnection
+ Compatibility with CommonUI for PlayStation icons
+ Works only on Windows

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
