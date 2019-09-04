//
// Created by Constantin on 8/10/2018.
//

#ifndef FPV_VR_TEXTELEMENTSWARNING_H
#define FPV_VR_TEXTELEMENTSWARNING_H

#include <vector>
#include <General/IDrawable.hpp>
#include <General/IPositionable.hpp>
#include <General/IUpdateable.hpp>
#include <General/ITextHeight.h>
#include <Helper/BasicGLPrograms.hpp>
#include <TelemetryReceiver.h>
#include <General/PositionDebug.hpp>
#include <OSD/ElementBatching/BatchingManager.h>
#include <OSD/ATextElements/OSDTextObj.hpp>


class TEWarning : public IDrawable, public IPositionable, public IUpdateable, public ITextHeight{
public:
    struct Options {
        bool batteryPercentage=false;
        bool batteryVoltage=false;
        bool batteryMAHUsed=false;
        const bool isEnabled()const{
            return (batteryPercentage || batteryVoltage || batteryMAHUsed);
        }
    };
    TEWarning(const TEWarning::Options& options,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,TelemetryReceiver& telemetryReceiver);
    Rect2D calculatePosition(const Rect2D &osdOverlay);
private:
    void setupPosition() override;
    void updateGL() override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    const TelemetryReceiver& mTelemetryReceiver;
    const Options& mOptions;
    PositionDebug mPositionDebug;
    static constexpr const wchar_t* MAX_TEXT_LENGTH_REFERENCE=L"W BATT VOLTAGE";
    static constexpr const int MAX_N_TEXT_OBJ=3; //Max. 3 warnings supported
    const std::vector<std::unique_ptr<OSDTextObj>> mGLTextObjIndices;
    static constexpr const int N_CHARS_PER_TEXT_OBJ=15;
};


#endif //FPV_VR_TEXTELEMENTSWARNING_H
