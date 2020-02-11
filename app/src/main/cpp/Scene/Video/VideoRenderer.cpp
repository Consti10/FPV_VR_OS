
#include <Color/Color.hpp>
#include <GeometryBuilder/ColoredGeometry.hpp>
#include <GeometryBuilder/TexturedGeometry.hpp>
#include <GeometryBuilder/SphereBuilder.hpp>
#include "VideoRenderer.h"
#include "Helper/GLHelper.hpp"

constexpr auto TAG="VideoRenderer";

VideoRenderer::VideoRenderer(VIDEO_RENDERING_MODE mode,const GLuint videoTexture,const GLProgramVC& glRenderGeometry,GLProgramTexture *glRenderTexEx):
mVideoTexture(videoTexture),
mMode(mode),mPositionDebug(glRenderGeometry,6, false),mGLRenderGeometry(glRenderGeometry){
    mGLRenderTexEx=glRenderTexEx;
    switch (mMode){
        case RM_NORMAL:
            mVideoCanvasB.initializeGL();
            break;
        case RM_STEREO:
            mVideoCanvasLeftEyeB.initializeGL();
            mVideoCanvasRightEyeB.initializeGL();
            break;
        case RM_360_EQUIRECTANGULAR:
            mEquirectangularSphereB.initializeGL();
            mInsta360SphereB.initializeGL();
            break;
    }
}

void VideoRenderer::updatePosition(const glm::vec3& lowerLeftCorner,const float width,const float height,
        const int optionalVideoWidthPx,const int optionalVideoHeightPx) {
    if(mMode==RM_NORMAL){
        const auto vid0=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                    width,height, TESSELATION_FACTOR, 0.0f,
                                                                    1.0f);
        mVideoCanvasB.initializeAndUploadGL(vid0.first,vid0.second);
    }else if(mMode==RM_STEREO){
        const auto vid1=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                    width,height, TESSELATION_FACTOR, 0.0f,
                                                                    0.5f);
        mVideoCanvasLeftEyeB.initializeAndUploadGL(vid1.first,vid1.second);
        const auto vid2=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                     width,height, TESSELATION_FACTOR, 0.5f,
                                                                     0.5f);
        mVideoCanvasRightEyeB.initializeAndUploadGL(vid2.first,vid2.second);
    }else if(mMode==RM_360_EQUIRECTANGULAR){
        //We need to recalculate the sphere u,v coordinates when the video ratio changes
        mEquirectangularSphereB.uploadGL(SphereBuilder::createSphereEquirectangularMonoscopic(),GL_TRIANGLE_STRIP);
        mInsta360SphereB.uploadGL(SphereBuilder::createSphereDualFisheyeInsta360(),GL_TRIANGLE_STRIP);
    }
}

void VideoRenderer::drawVideoCanvas(glm::mat4x4 ViewM, glm::mat4x4 ProjM, bool leftEye) {
    if(mMode==RM_360_EQUIRECTANGULAR){
        drawVideoCanvas360(ViewM,ProjM);
    }else if(mMode==RM_NORMAL || mMode==RM_STEREO){
        const auto buff=mMode==RM_NORMAL ? mVideoCanvasB : leftEye ? mVideoCanvasLeftEyeB : mVideoCanvasRightEyeB;
        mGLRenderTexEx->drawX(mVideoTexture,ViewM,ProjM,buff);
    }
    //We render the debug rectangle after the other one such that it always appears when enabled (overdraw)
    mPositionDebug.drawGLDebug(ViewM,ProjM);
    GLHelper::checkGlError("VideoRenderer::drawVideoCanvas");
}

void VideoRenderer::drawVideoCanvas360(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
    if(mMode!=VIDEO_RENDERING_MODE::RM_360_EQUIRECTANGULAR){
        throw "mMode!=VIDEO_RENDERING_MODE::Degree360";
    }
    const float scale=200.0f;
    glm::mat4 scaleM=glm::scale(glm::vec3(scale,scale,scale));
    mGLRenderTexEx->drawX(mVideoTexture,ViewM*scaleM,ProjM,mEquirectangularSphereB);
    GLHelper::checkGlError("VideoRenderer::drawVideoCanvas360");
}






