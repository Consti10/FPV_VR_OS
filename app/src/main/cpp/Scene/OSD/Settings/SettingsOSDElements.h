//
// Created by Constantin on 1/17/2019.
//

#ifndef FPV_VR_OSDSETTINGS_H
#define FPV_VR_OSDSETTINGS_H

#include <string>
#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <android/log.h>
#include <TelemetryReceiver.h>

#include <OSD/Ladders/CompassLadder.h>
#include <OSD/Text/TEWarning.h>
#include <OSD/Text/TextElements1.h>
#include <OSD/Text/TextElements2.h>
#include <OSD/ArtificialHorizon/AHorizon.h>
#include <Color/Color.hpp>
#include <OSD/Ladders/VLAltitude.h>
#include <OSD/Ladders/VLSpeed.h>

class SettingsOSDElements {
public:
    SettingsOSDElements(JNIEnv *env, jobject androidContext);
    SettingsOSDElements(SettingsOSDElements const &) = delete;
    void operator=(SettingsOSDElements const &)= delete;
public:
//Options for each OSD element
    AHorizon::Options oArtificialHorizon;
    CompassLadder::Options oCompassL;
    VLAltitude::Options oAltitudeL;
    VLSpeed::Options oSpeedL;
    TEWarning::Options oTextWarning;
    TextElements1::Options oTextElement1;
    TextElements2::Options oTextElement2;
};


#endif //FPV_VR_OSDSETTINGS_H
