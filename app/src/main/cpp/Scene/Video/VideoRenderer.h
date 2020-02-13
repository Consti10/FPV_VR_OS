#ifndef CONSTI10_FPV_VR_OS_VIDEO_RENDERER
#define CONSTI10_FPV_VR_OS_VIDEO_RENDERER

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GLProgramTexture.h>
#include <GLProgramVC.h>
#include <Helper/GLBufferHelper.hpp>


class VideoRenderer{ //Does not inherit from IDrawable !
public:
    //Normal: Render a rectangle
    //Stereo: One decoded frame contains images for left and right eye
    //Render left frame into a rectangle with u->{0,0,5} and right frame int a rectangle with u->{0.5,1.0}
    //Degree360: 360 degree video, rendered onto a sphere instead of a quad
    //The daydream renderer handles external surfaces (like video) itself, but requires the application to
    //RM_PunchHole 'punch a hole' into the scene by rendering a quad with alpha=0.0f deprecated
    enum VIDEO_RENDERING_MODE{RM_2D_MONOSCOPIC,RM_2D_STEREO,RM_360_DUAL_FISHEYE_INSTA1,RM_360_DUAL_FISHEYE_INSTA2};
    /*
     * @param VIDEO_RENDERING_MODE one of the rendering modes above
     * @param videoTexture a valid OpenGL texture
     * @param DistortionManager: optional, used to create GLProgramTextureExt if needed
     */
    VideoRenderer(VIDEO_RENDERING_MODE mode,const GLuint videoTexture,const DistortionManager* distortionManager);
    void drawVideoCanvas(glm::mat4x4 ViewM, glm::mat4x4 ProjM, bool leftEye);
    void drawVideoCanvas360(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    //For 360 equirectangular we need the video vidth and height in px
    void updatePosition(const glm::vec3& lowerLeftCorner,const float width,const float height,int optionalVideoWidthPx,int optionalVideoHeightPx);
    bool is360Video(){
        return mMode==RM_360_DUAL_FISHEYE_INSTA1 || mMode==RM_360_DUAL_FISHEYE_INSTA2;
    }
private:
    VertexBuffer mEquirectangularSphereB; //Equirectangular Sphere
    VertexBuffer mInsta360SphereB;
    VertexIndexBuffer mVideoCanvasB;//whole video frame (u.v coordinates). Tesselated
    VertexIndexBuffer mVideoCanvasLeftEyeB;//left side of the video frame (u.v coordinates) Tesselated
    VertexIndexBuffer mVideoCanvasRightEyeB; //right side of the video frame (u.v coordinates) Tesselated

    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExtMappingEnabled;

    const int TESSELATION_FACTOR=10;
    const VIDEO_RENDERING_MODE mMode;
    const GLuint mVideoTexture;
};

#endif //CONSTI10_FPV_VR_OS_VIDEO_RENDERER
