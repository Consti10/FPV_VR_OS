# FPV_VR_OS
Latest open-sourced version of FPV_VR

<img src="https://github.com/Consti10/FPV_VR_OS/blob/master/screenshots/ss1_originalSettings.png" alt="ExampleMain" width="240">

[Buy app here](https://play.google.com/store/apps/details?id=constantin.fpv_vr.wifibroadcast&hl=en) to support development


**Build Instructions** \
This library depends on 3 modules that are published in other github repositories. \
These are:
1. VideoCore / TelemetryCore from [LiveVIdeo10ms](https://github.com/Consti10/LiveVideo10ms)
3. RenderingXCore from [RenderingX](https://github.com/Consti10/RenderingX)

To Build the FPV_VR app you also have to download these repositories and provide the right paths.\
By default, FPV-VR is configured such that it will compile out of the box in the following file structure: \
**+AndroidStudioProjects \
--+FPV_VR_OS \
--+LiveVIdeo10ms \
--+RenderingX** \

E.g execute the following commands one after each other in the same folder: \
git clone https://github.com/Consti10/FPV_VR_OS \
git clone https://github.com/Consti10/LiveVideo10ms \
git clone https://github.com/Consti10/RenderingX \

Then Open FPV_VR_OS via the Android Studio 'open File or Project' wizzard.