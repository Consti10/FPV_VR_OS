#include "OSDRenderer.h"
#include <BasicGLPrograms.hpp>
#include <GLHelper.hpp>
#include <TimeHelper.hpp>


OSDRenderer::OSDRenderer(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver):
        settingsOSDStyle(env,androidContext),
        settingsOSDElements(env,androidContext),
        mBasicGLPrograms(),
        mBatchingManager(mBasicGLPrograms),
        mTelemetryReceiver(telemetryReceiver){
    //
    mBasicGLPrograms.text.loadTextRenderingData(env, androidContext,settingsOSDStyle.OSD_TEXT_FONT_TYPE);

    if(settingsOSDElements.oTextElement1.isEnabled()){
        mTextElements1=new TextElements1(settingsOSDElements.oTextElement1,settingsOSDStyle,mBasicGLPrograms,mBatchingManager,telemetryReceiver);
        mDrawables.push_back(mTextElements1);
        mUpdateables.push_back(mTextElements1);
    }
    if(!settingsOSDElements.OSD_DISABLE_ALL_OVERLAY_ELEMENTS){
        if(settingsOSDElements.oArtificialHorizon.isEnabled()){
            mAHorizon=new AHorizon(settingsOSDElements.oArtificialHorizon,settingsOSDStyle,mBasicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mAHorizon);
            mUpdateables.push_back(mAHorizon);
        }
        if(settingsOSDElements.oCompassL.enable){
            mCompassLadder=new CompassLadder(settingsOSDElements.oCompassL,settingsOSDStyle,mBasicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mCompassLadder);
            mUpdateables.push_back(mCompassLadder);
        }
        if(settingsOSDElements.oAltitudeL.enable){
            mAltitudeLadder=new VLAltitude(settingsOSDElements.oAltitudeL,settingsOSDStyle,mBasicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mAltitudeLadder);
            mUpdateables.push_back(mAltitudeLadder);
        }
        if(settingsOSDElements.oSpeedL.enable){
            mSpeedLadder=new VLSpeed(settingsOSDElements.oSpeedL,settingsOSDStyle,mBasicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mSpeedLadder);
            mUpdateables.push_back(mSpeedLadder);
        }
        if(settingsOSDElements.oTextElement2.isEnabled()){
            mTextElements2=new TextElements2(settingsOSDElements.oTextElement2,settingsOSDStyle,mBasicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mTextElements2);
            mUpdateables.push_back(mTextElements2);
        }
        if(settingsOSDElements.oTextWarning.isEnabled()){
            mTEWarning=new TEWarning(settingsOSDElements.oTextWarning,mBasicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mTEWarning);
            mUpdateables.push_back(mTEWarning);
        }
    }
    mFLightStart=std::chrono::steady_clock::now();
}

void OSDRenderer::placeLOL(const int widthPx,const int heightPx) {
    const float ratio=(float)heightPx/(float)widthPx;
    const float videoZ=0;
    const float videoW=widthPx;//
    const float videoH=heightPx; //videoW*ratio
    //glm::tan(glm::radians(45.0f)*0.5f)*1*2;
    //MLOGD<<"W H "<<widthPx<<" "<<heightPx<<"ratio "<<ratio;
   // float videoH=1;
    const float videoX=-videoW/2.0f;
    const float videoY=-videoH/2.0f;

    const float textHeightMono=videoH*0.045f*(settingsOSDStyle.OSD_MONO_GLOBAL_SCALE*0.01f)*
                               (settingsOSDElements.oTextElement1.scale*0.01f);
    float te1HeightTop=0;
    float te1HeightBottom=0;
    if(mTextElements1){
        mTextElements1->setTextHeight(textHeightMono);
        mTextElements1->setWorldPosition(videoX, videoY, videoZ, videoW, videoH);
        glm::ivec3 cteH= mTextElements1->calculateRowsAndColumns(videoW);
        te1HeightTop=textHeightMono*cteH[1];
        te1HeightBottom=textHeightMono*cteH[2];
    }
    //The space needed by mTextElements1 is dynamic. When no optional value is specified, it might
    //take zero space, but it can hold up to 2 rows of text in the top and bottom
    //after we have placed this element, we can specify our new rectangle, that has the same width,
    //X and Z values, but different height and Y values
    const IPositionable::Rect2D rectOSDOverlay(videoX,videoY+te1HeightBottom,videoZ,
                                               videoW,videoH-te1HeightTop-te1HeightBottom);
    if(mTextElements2){
        mTextElements2->setTextHeight(textHeightMono);
        mTextElements2->setWorldPosition(rectOSDOverlay);
    }
    if(mTEWarning){
        mTEWarning->setTextHeight(textHeightMono);
        mTEWarning->setWorldPosition(mTEWarning->calculatePosition(rectOSDOverlay));
    }
    if(mCompassLadder){
        mCompassLadder->setWorldPosition(mCompassLadder->calculatePosition(rectOSDOverlay,false));
    }
    if(mAltitudeLadder){
        mAltitudeLadder->setWorldPosition(mAltitudeLadder->calculatePosition(rectOSDOverlay,false));
    }
    if(mSpeedLadder){
        mSpeedLadder->setWorldPosition(mSpeedLadder->calculatePosition(rectOSDOverlay,false));
    }
    if(mAHorizon){
        mAHorizon->setWorldPosition(mAHorizon->calculatePosition(rectOSDOverlay,settingsOSDStyle.OSD_MONO_GLOBAL_SCALE));
    }
    mBatchingManager.initGL();
    mBatchingManager.setTextColor(settingsOSDStyle.OSD_TEXT_OUTLINE_COLOR,settingsOSDStyle.OSD_MONO_TEXT_OUTLINE_STRENGTH);
    GLHelper::checkGlError("OSDRenderer::placeGLElementsMono");
    mFLightStart=std::chrono::steady_clock::now();
    //
    //mOSDProjectionM=glm::perspective(glm::radians(45.0f),ratio,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    //mOSDProjectionM=glm::ortho(0,1920,0,1080);
    // Simple ortographic projection that transforms pixel coordinates into OpenGL NDC coordinates
    // For example range (0,0)(640,480) to (-1,-1) and (1,1)
    mOSDProjectionM=glm::ortho(-videoW/2.0f,videoW/2.0f,-videoH/2.0f,videoH/2.0f);
    //mOSDProjectionM=glm::mat4(1.0f);
}

void OSDRenderer::updateAndDrawElementsGL(){
    //MEASURE_FUNCTION_EXECUTION_TIME
    const auto now=std::chrono::steady_clock::now();
    const float flightTimeS=((float)std::chrono::duration_cast<std::chrono::milliseconds>(now-mFLightStart).count())/1000.0f;
    mTelemetryReceiver.setFlightTime(flightTimeS);
    IUpdateable::updateAll(mUpdateables);
    mBatchingManager.updateGL();
    mBatchingManager.drawGL(IDENTITY_M,mOSDProjectionM);
    IDrawable::drawAll(mDrawables,IDENTITY_M,mOSDProjectionM);
    GLHelper::checkGlError("OSDRenderer::updateAndDrawElementsGL");
    //LOGD("Valid %d %d %d %d %d %d",mTextElements1,mTextElements2,mTEWarning,mAHorizon,mAltitudeLadder,mSpeedLadder);
}




