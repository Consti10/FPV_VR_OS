#include "OSDRenderer.h"
#include <BasicGLPrograms.hpp>
#include <GLHelper.hpp>
#include <TimeHelper.hpp>


OSDRenderer::OSDRenderer(JNIEnv* env,jobject androidContext,const BasicGLPrograms& basicGLPrograms,TelemetryReceiver& telemetryReceiver):
        settingsOSDStyle(env,androidContext),
        settingsOSDElements(env,androidContext),
        mBatchingManager(basicGLPrograms),
        mTelemetryReceiver(telemetryReceiver){
    if(settingsOSDElements.oTextElement1.isEnabled()){
        mTextElements1=new TextElements1(settingsOSDElements.oTextElement1,settingsOSDStyle,basicGLPrograms,mBatchingManager,telemetryReceiver);
        mDrawables.push_back(mTextElements1);
        mUpdateables.push_back(mTextElements1);
    }
    if(!settingsOSDElements.OSD_DISABLE_ALL_OVERLAY_ELEMENTS){
        if(settingsOSDElements.oArtificialHorizon.isEnabled()){
            mAHorizon=new AHorizon(settingsOSDElements.oArtificialHorizon,settingsOSDStyle,basicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mAHorizon);
            mUpdateables.push_back(mAHorizon);
        }
        if(settingsOSDElements.oCompassL.enable){
            mCompassLadder=new CompassLadder(settingsOSDElements.oCompassL,settingsOSDStyle,basicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mCompassLadder);
            mUpdateables.push_back(mCompassLadder);
        }
        if(settingsOSDElements.oAltitudeL.enable){
            mAltitudeLadder=new VLAltitude(settingsOSDElements.oAltitudeL,settingsOSDStyle,basicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mAltitudeLadder);
            mUpdateables.push_back(mAltitudeLadder);
        }
        if(settingsOSDElements.oSpeedL.enable){
            mSpeedLadder=new VLSpeed(settingsOSDElements.oSpeedL,settingsOSDStyle,basicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mSpeedLadder);
            mUpdateables.push_back(mSpeedLadder);
        }
        if(settingsOSDElements.oTextElement2.isEnabled()){
            mTextElements2=new TextElements2(settingsOSDElements.oTextElement2,settingsOSDStyle,basicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mTextElements2);
            mUpdateables.push_back(mTextElements2);
        }
        if(settingsOSDElements.oTextWarning.isEnabled()){
            mTEWarning=new TEWarning(settingsOSDElements.oTextWarning,basicGLPrograms,mBatchingManager,telemetryReceiver);
            mDrawables.push_back(mTEWarning);
            mUpdateables.push_back(mTEWarning);
        }
    }
    mFLightStart=std::chrono::steady_clock::now();
}

/**
 * In mono mode we get a rectangle that exactly fills the screen
 */
void OSDRenderer::placeGLElementsMono(const IPositionable::Rect2D& rectViewport) {
    const float textHeightMono=rectViewport.mHeight*0.045f*(settingsOSDStyle.OSD_MONO_GLOBAL_SCALE*0.01f)*
            (settingsOSDElements.oTextElement1.scale*0.01f);
    float te1HeightTop=0;
    float te1HeightBottom=0;
    if(mTextElements1){
        mTextElements1->setTextHeight(textHeightMono);
        mTextElements1->setWorldPosition(rectViewport.mX, rectViewport.mY, rectViewport.mZ, rectViewport.mWidth, rectViewport.mHeight);
        glm::ivec3 cteH= mTextElements1->calculateRowsAndColumns(rectViewport.mWidth);
        te1HeightTop=textHeightMono*cteH[1];
        te1HeightBottom=textHeightMono*cteH[2];
    }
    //The space needed by mTextElements1 is dynamic. When no optional value is specified, it might
    //take zero space, but it can hold up to 2 rows of text in the top and bottom
    //after we have placed this element, we can specify our new rectangle, that has the same width,
    //X and Z values, but different height and Y values
    const IPositionable::Rect2D rectOSDOverlay(rectViewport.mX,rectViewport.mY+te1HeightBottom,rectViewport.mZ,
                                               rectViewport.mWidth,rectViewport.mHeight-te1HeightTop-te1HeightBottom);
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
}

/**
 * In stereo mode we get the rectangle the video is rendered onto
 */
void OSDRenderer::placeGLElementsStereo(const IPositionable::Rect2D& rectVideoCanvas) {
    const float textHeightStereo=rectVideoCanvas.mWidth*0.03f*(settingsOSDStyle.OSD_STEREO_GLOBAL_SCALE*0.01f)*
                                                         (settingsOSDElements.oTextElement1.scale*0.01f);
    float paddingX=rectVideoCanvas.mWidth-rectVideoCanvas.mWidth*(settingsOSDStyle.OSD_STEREO_FOVX_SCALE*0.01f);
    float paddingY=rectVideoCanvas.mHeight-rectVideoCanvas.mHeight*(settingsOSDStyle.OSD_STEREO_FOVY_SCALE*0.01f);
    IPositionable::Rect2D rectOSDOverlay(rectVideoCanvas.mX+paddingX/2.0f,rectVideoCanvas.mY+paddingY/2.0f,rectVideoCanvas.mZ,
                                        rectVideoCanvas.mWidth-paddingX,rectVideoCanvas.mHeight-paddingY);

    MLOGD<<"OSD rect"<<rectOSDOverlay.mWidth/rectOSDOverlay.mHeight;

    if(mTextElements1){
        mTextElements1->setTextHeight(textHeightStereo);
        mTextElements1->setWorldPosition(mTextElements1->calculatePositionStereo(rectOSDOverlay));
    }
    if(mTextElements2){
        mTextElements2->setTextHeight(textHeightStereo);
        mTextElements2->setWorldPosition(rectOSDOverlay);
    }
    if(mTEWarning){
        mTEWarning->setTextHeight(textHeightStereo);
        mTEWarning->setWorldPosition(mTEWarning->calculatePosition(rectOSDOverlay));
    }
    if(mCompassLadder){
        mCompassLadder->setWorldPosition(mCompassLadder->calculatePosition(rectOSDOverlay,true));
    }
    if(mAltitudeLadder){
        mAltitudeLadder->setWorldPosition(mAltitudeLadder->calculatePosition(rectOSDOverlay,true));
    }
    if(mSpeedLadder){
        mSpeedLadder->setWorldPosition(mSpeedLadder->calculatePosition(rectOSDOverlay,true));
    }
    if(mAHorizon){
        mAHorizon->setWorldPosition(mAHorizon->calculatePosition(rectOSDOverlay,settingsOSDStyle.OSD_STEREO_GLOBAL_SCALE));
    }
    mBatchingManager.initGL();
    mBatchingManager.setTextColor(settingsOSDStyle.OSD_TEXT_OUTLINE_COLOR,settingsOSDStyle.OSD_STEREO_TEXT_OUTLINE_STRENGTH);
    GLHelper::checkGlError("OSDRenderer::placeGLElementsStereo");
    mFLightStart=std::chrono::steady_clock::now();
}

void OSDRenderer::updateAndDrawElementsGL(glm::mat4 ViewM,glm::mat4 ProjM){
    //MEASURE_FUNCTION_EXECUTION_TIME
    const auto now=std::chrono::steady_clock::now();
    const float flightTimeS=((float)std::chrono::duration_cast<std::chrono::milliseconds>(now-mFLightStart).count())/1000.0f;
    mTelemetryReceiver.setFlightTime(flightTimeS);
    IUpdateable::updateAll(mUpdateables);
    mBatchingManager.updateGL();
    mBatchingManager.drawGL(ViewM,ProjM);
    IDrawable::drawAll(mDrawables,ViewM,ProjM);
    GLHelper::checkGlError("OSDRenderer::updateAndDrawElementsGL");
    //LOGD("Valid %d %d %d %d %d %d",mTextElements1,mTextElements2,mTEWarning,mAHorizon,mAltitudeLadder,mSpeedLadder);
}

void OSDRenderer::drawElementsGL(glm::mat4 ViewM, glm::mat4 ProjM) {
    //MEASURE_FUNCTION_EXECUTION_TIME
    mBatchingManager.drawGL(ViewM,ProjM);
    IDrawable::drawAll(mDrawables,ViewM,ProjM);
    GLHelper::checkGlError("OSDRenderer::drawElementsGL");
}

void OSDRenderer::placeLOL(const float ratio) {
    const float videoRatio=4.0f/3.0f;
    float videoZ=-10;
    float videoH=glm::tan(glm::radians(45.0f)*0.5f)*10*2;
    float videoW=videoH*ratio;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;

    placeGLElementsMono(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));

    projMatrixMono=glm::perspective(glm::radians(45.0f),ratio,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
}



