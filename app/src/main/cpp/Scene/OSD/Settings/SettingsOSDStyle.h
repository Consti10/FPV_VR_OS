//
// Created by Constantin on 2/3/2019.
//

#ifndef FPV_VR_SETTINGSOSDSTYLE_H
#define FPV_VR_SETTINGSOSDSTYLE_H


#include <jni.h>
#include <TrueColor.hpp>
#include <TextAssetsHelper.hpp>

class SettingsOSDStyle {
public:
    SettingsOSDStyle(JNIEnv *env, jobject androidContext,int SURFACE_H_PX);
    SettingsOSDStyle(SettingsOSDStyle const &) = delete;
    void operator=(SettingsOSDStyle const &)= delete;
    static TrueColor getOSDBackgroundColor(int _alpha){
        float alpha=_alpha*0.01f;
        if(alpha<0 || alpha>1){alpha=0;}
        return TrueColor(glm::vec4(0.0f, 0.0f, 0.0f, alpha));
    }
    static bool isTransparentBackgroundEnabled(int _alpha){
        return _alpha>0;
    }
public:
    //----STYLE---
    TrueColor OSD_LINE_FILL_COLOR;
    TrueColor OSD_LINE_OUTLINE_COLOR;
    //
    //int OSD_TEXT_FONT_TYPE;
    TextAssetsHelper::TEXT_STYLE  OSD_TEXT_FONT_TYPE;
    TrueColor OSD_TEXT_FILL_COLOR1;
    TrueColor OSD_TEXT_FILL_COLOR2;
    TrueColor OSD_TEXT_FILL_COLOR3;
    TrueColor OSD_TEXT_OUTLINE_COLOR;
    float OSD_MONO_TEXT_OUTLINE_STRENGTH;
    float OSD_STEREO_TEXT_OUTLINE_STRENGTH;
    int OSD_TRANSPARENT_BACKGROUND_STRENGTH;
    //
    int OSD_MONO_GLOBAL_SCALE;
    int OSD_STEREO_GLOBAL_SCALE;
public:
    float OSD_STEREO_RATIO;
public:
    // set manually depending on the resolution of the Surface the OSD is rendered onto
    int OSD_LINE_WIDTH_PX_1; // small line width
    int OSD_LINE_WIDTH_PX_2;// big line width
    void calculateLineWidth(int SURFACE_H_PX){
        if(SURFACE_H_PX<=720){
            // aka really low resolution
            OSD_LINE_WIDTH_PX_1=1;
            OSD_LINE_WIDTH_PX_2=2;
        }else if(SURFACE_H_PX<=1080){
            // aka medium/higher resolution
            OSD_LINE_WIDTH_PX_1=3;
            OSD_LINE_WIDTH_PX_2=4;
        }else{
            // aka really high resolution
            OSD_LINE_WIDTH_PX_1=3;
            OSD_LINE_WIDTH_PX_2=4;
        }
    }
};


#endif //FPV_VR_SETTINGSOSDSTYLE_H
