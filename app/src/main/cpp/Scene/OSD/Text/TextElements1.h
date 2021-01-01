//
// Created by Constantin on 6/10/2018.
//

#ifndef FPV_VR_OSD_TEXT_ELEMENTS_1_H
#define FPV_VR_OSD_TEXT_ELEMENTS_1_H

#include <vector>
#include <GLES2/gl2.h>
#include <GLProgramText.h>
#include <GLProgramVC.h>
#include <TelemetryReceiver.h>
#include <thread>
#include <atomic>
#include <BasicGLPrograms.hpp>
#include <General/ITextHeight.h>
#include "OSD/ATextElements/OSDTextObj.hpp"
#include "../../General/IPositionable.hpp"
#include "../../General/IDrawable.hpp"
#include "General/IUpdateable.hpp"

// All sub-elements are ordered in row/column fashion
class TextElements1: public IDrawable,public IPositionable,public IUpdateable, public ITextHeight{
public:
    struct Options{ // Options for the custom text element
        std::vector<TelemetryReceiver::TelemetryValueIndex> enableXX={};
        const bool isEnabled()const{
            return !enableXX.empty();
        }
        int scale=100;
    };
    TextElements1(const TextElements1::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,TelemetryReceiver& telemetryR);
    Rect2D calculatePositionStereo(const Rect2D &osdOverlayCanvas);
    glm::ivec3 calculateRowsAndColumns(float width);
private:
    void setupPosition() override;
    void updateGL() override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    void updateSubElement(unsigned long id,OSDTextObj* obj)const;
    const Options& mOptions;
    const SettingsOSDStyle& settingsOSDStyle;
    const TelemetryReceiver& mTelemetryR;
    const std::vector<std::unique_ptr<OSDTextObj>> mGLTextObjIndices;
    static constexpr const wchar_t* MAX_TEXT_LENGTH_REFERENCE=L"Lat:99.9999999";
    static constexpr const int N_CHARS_PER_TEXT_OBJ=15;
    static std::vector<std::unique_ptr<OSDTextObj>> allocateAllElements(const SettingsOSDStyle& settingsOSDStyle,const Options& options,BatchingManager &batchingManager);
};

#endif //FPV_VR_OSD_TEXT_ELEMENTS_1_H
