
#include <Color/Color.hpp>
#include <GeometryBuilder/ColoredGeometry.hpp>
#include <GeometryBuilder/TexturedGeometry.hpp>
#include <GeometryBuilder/SphereBuilder.hpp>
#include "VideoRenderer.h"
#include "Helper/GLHelper.hpp"

constexpr auto TAG="VideoRenderer";

VideoRenderer::VideoRenderer(VIDEO_RENDERING_MODE mode,const GLuint videoTexture,const DistortionManager* distortionManager):
mVideoTexture(videoTexture),mMode(mode){
    mGLProgramTextureExtMappingEnabled=std::make_unique<GLProgramTextureExt>(distortionManager,true);
    mGLProgramTextureExt=std::make_unique<GLProgramTextureExt>(distortionManager,false);
    switch (mMode){
        case RM_2D_MONOSCOPIC:
            mVideoCanvasB.initializeGL();
            break;
        case RM_2D_STEREO:
            mVideoCanvasLeftEyeB.initializeGL();
            mVideoCanvasRightEyeB.initializeGL();
            break;
        case RM_360_DUAL_FISHEYE_INSTA1:
        case RM_360_DUAL_FISHEYE_INSTA2:
            mEquirectangularSphereB.initializeGL();
            mInsta360SphereB.initializeGL();
            mEquirectangularSphereB.uploadGL(SphereBuilder::createSphereEquirectangularMonoscopic(),GL_TRIANGLE_STRIP);
            mInsta360SphereB.uploadGL(SphereBuilder::createSphereDualFisheyeInsta360(),GL_TRIANGLE_STRIP);
            break;
    }
}

void VideoRenderer::updatePosition(const glm::vec3& lowerLeftCorner,const float width,const float height,
        const int optionalVideoWidthPx,const int optionalVideoHeightPx) {
    if(mMode==RM_2D_MONOSCOPIC){
        const auto vid0=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                    width,height, TESSELATION_FACTOR, 0.0f,
                                                                    1.0f);
        mVideoCanvasB.initializeAndUploadGL(vid0.first,vid0.second);
    }else if(mMode==RM_2D_STEREO){
        const auto vid1=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                    width,height, TESSELATION_FACTOR, 0.0f,
                                                                    0.5f);
        mVideoCanvasLeftEyeB.initializeAndUploadGL(vid1.first,vid1.second);
        const auto vid2=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                     width,height, TESSELATION_FACTOR, 0.5f,
                                                                     0.5f);
        mVideoCanvasRightEyeB.initializeAndUploadGL(vid2.first,vid2.second);
    }//else if(false){
        //We need to recalculate the sphere u,v coordinates when the video ratio changes
        //mEquirectangularSphereB.uploadGL(SphereBuilder::createSphereEquirectangularMonoscopic(),GL_TRIANGLE_STRIP);
        //mInsta360SphereB.uploadGL(SphereBuilder::createSphereDualFisheyeInsta360(),GL_TRIANGLE_STRIP);
    //}
}

void VideoRenderer::drawVideoCanvas(glm::mat4x4 ViewM, glm::mat4x4 ProjM, bool leftEye) {
    if(mMode==RM_360_DUAL_FISHEYE_INSTA1 || mMode==RM_360_DUAL_FISHEYE_INSTA2){
        drawVideoCanvas360(ViewM,ProjM);
    }else if(mMode==RM_2D_MONOSCOPIC || mMode==RM_2D_STEREO){
        const auto buff=mMode==RM_2D_MONOSCOPIC ? mVideoCanvasB : leftEye ? mVideoCanvasLeftEyeB : mVideoCanvasRightEyeB;
        mGLProgramTextureExt->drawX(mVideoTexture,ViewM,ProjM,buff);
    }
    GLHelper::checkGlError("VideoRenderer::drawVideoCanvas");
}

void VideoRenderer::drawVideoCanvas360(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
    if(!(is360Video())){
        throw "mMode!=VIDEO_RENDERING_MODE::Degree360";
    }
    const float scale=100.0f;
    const glm::mat4 scaleM=glm::scale(glm::vec3(scale,scale,scale));
    const glm::mat4x4 modelMatrix=glm::rotate(glm::mat4(1.0F),glm::radians(90.0F), glm::vec3(0,0,-1))*scaleM;
    if(mMode==RM_360_DUAL_FISHEYE_INSTA1){
        mGLProgramTextureExtMappingEnabled->drawX(mVideoTexture,ViewM*modelMatrix,ProjM,mEquirectangularSphereB);
    }else{
        mGLProgramTextureExt->drawX(mVideoTexture,ViewM*modelMatrix,ProjM,mInsta360SphereB);
    }
    GLHelper::checkGlError("VideoRenderer::drawVideoCanvas360");
}






