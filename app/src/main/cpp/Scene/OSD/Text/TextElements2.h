//
// Created by Constantin on 6/10/2018.
//


#ifndef FPV_VR_OSD_TEXT_ELEMENTS_2_H
#define FPV_VR_OSD_TEXT_ELEMENTS_2_H

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramText.h>
#include <GLProgramVC.h>
#include <TelemetryReceiver.h>
#include <BasicGLPrograms.hpp>
#include <General/ITextHeight.h>
#include "OSD/ATextElements/OSDTextObj.hpp"
#include "../../General/IPositionable.hpp"
#include "../../General/IDrawable.hpp"
#include "../../General/PositionDebug.hpp"
#include "General/IUpdateable.hpp"

// All Sub-elements have fixed positions (upper right, lower right, lower middle usw)
class TextElements2 : public IDrawable,public IPositionable,public IUpdateable, public ITextHeight {
public:
    struct Options {
        std::vector<TelemetryReceiver::TelemetryValueIndex > enableXX={};
        const bool isEnabled()const{
            return !enableXX.empty();
        }
        int scale=100;
    };
    TextElements2(const TextElements2::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,TelemetryReceiver& telemetryReceiver);
private:
    void setupPosition() override;
    void updateGL() override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    void updateSubElement(unsigned long id,OSDTextObj* obj)const;
    static std::vector<std::unique_ptr<OSDTextObj>> createAllElements(const SettingsOSDStyle& settingsOSDStyle,BatchingManager &batchingManager,const TextElements2::Options& options);
    const Options& mOptions;
    const TelemetryReceiver& mTelemetryReceiver;
    const std::vector<std::unique_ptr<OSDTextObj>> mGLTextObjIndices;
    PositionDebug mPositionDebug;
    const SettingsOSDStyle& settingsOSDStyle;

    const wchar_t* MAX_TEXT_LENGTH_REFERENCE_2=L"Lon 99.9999999";
    const wchar_t* MAX_TEXT_LENGTH_REFERENCE_2_1=L"VS 0.00 km/h";
    const wchar_t* MAX_TEXT_LENGTH_REFERENCE_3=L"99.8 V";
    const wchar_t* MAX_TEXT_LENGTH_REFERENCE_EZWB_1=L"-99dBm ";
    const wchar_t* MAX_TEXT_LENGTH_REFERENCE_EZWB_2=L"99% ";
    const wchar_t* MAX_TEXT_LENGTH_REFERENCE_EZWB_3=L"-26dBm [10]";
    const wchar_t* MAX_TEXT_LENGTH_REFERENCE_FLIGHT_MODE=L"AxxxxxxxxxxA";

    static constexpr float TEXT_UPSCALE_FACTOR=1.3f;
    static constexpr float TEXT_DOWNSCALE_FACTOR=0.8f;
};


#endif //FPV_VR_OSD_TEXT_ELEMENTS_2_H
