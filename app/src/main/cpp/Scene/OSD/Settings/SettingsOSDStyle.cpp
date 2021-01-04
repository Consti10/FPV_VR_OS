//
// Created by Constantin on 2/3/2019.
//

#include "SettingsOSDStyle.h"
#include "IDOSD.hpp"
#include <android/log.h>
#include <SharedPreferences.hpp>

SettingsOSDStyle::SettingsOSDStyle(JNIEnv *env, jobject androidContext,int SURFACE_H_PX) {
//OSD style ------------
    SharedPreferences prefOSDStyle(env,androidContext,"pref_osd");
    OSD_LINE_FILL_COLOR= TrueColor::ARGB(prefOSDStyle.getInt(IDOSD::OSD_LINE_FILL_COLOR));
    OSD_LINE_OUTLINE_COLOR= TrueColor::ARGB(
            prefOSDStyle.getInt(IDOSD::OSD_LINE_OUTLINE_COLOR));
    OSD_TEXT_FONT_TYPE=static_cast<TextAssetsHelper::TEXT_STYLE>(prefOSDStyle.getInt(IDOSD::OSD_TEXT_FONT_TYPE));

    OSD_TEXT_FILL_COLOR1= TrueColor::ARGB(
            prefOSDStyle.getInt(IDOSD::OSD_TEXT_FILL_COLOR1));

    OSD_TEXT_FILL_COLOR2= TrueColor::ARGB(
            prefOSDStyle.getInt(IDOSD::OSD_TEXT_FILL_COLOR2));
    OSD_TEXT_FILL_COLOR3= TrueColor::ARGB(
            prefOSDStyle.getInt(IDOSD::OSD_TEXT_FILL_COLOR3));
    OSD_TEXT_OUTLINE_COLOR= TrueColor::ARGB(
            prefOSDStyle.getInt(IDOSD::OSD_TEXT_OUTLINE_COLOR));
    OSD_TRANSPARENT_BACKGROUND_STRENGTH=prefOSDStyle.getInt(IDOSD::OSD_TRANSPARENT_BACKGROUND_STRENGTH);
    OSD_MONO_GLOBAL_SCALE=prefOSDStyle.getInt(IDOSD::OSD_MONO_GLOBAL_SCALE);
    OSD_STEREO_GLOBAL_SCALE=prefOSDStyle.getInt(IDOSD::OSD_STEREO_GLOBAL_SCALE);
    OSD_MONO_TEXT_OUTLINE_STRENGTH=prefOSDStyle.getFloat(IDOSD::OSD_MONO_TEXT_OUTLINE_STRENGTH);
    OSD_STEREO_TEXT_OUTLINE_STRENGTH=prefOSDStyle.getFloat(IDOSD::OSD_STEREO_TEXT_OUTLINE_STRENGTH);
//OSD style ----------------------------
    //LOGD("%s",Color::asString(OSD_TEXT_FILL_COLOR1).c_str());
    //LOGD("%s",Color::asString(OSD_TEXT_FILL_COLOR2).c_str());
    //LOGD("%s",Color::asString(OSD_TEXT_FILL_COLOR3).c_str());
    //LOGD("%s",Color::asString(OSD_TEXT_OUTLINE_COLOR).c_str());

    /*OSD_TEXT_FILL_COLOR1=Color::fromRGBA(0,0,0,1);
    const auto tmp=Color::toRGBA(OSD_TEXT_FILL_COLOR1);
    OSD_TEXT_FILL_COLOR1=Color::fromRGBA(tmp.x,tmp.y,tmp.z,tmp.w);*/
    OSD_STEREO_RATIO=prefOSDStyle.getFloat(IDOSD::OSD_STEREO_RATIO,1.333f);
    // other
    calculateLineWidth(SURFACE_H_PX);
}