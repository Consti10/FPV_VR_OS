#ifndef CONSTI10_FPV_VR_OS_VIDEO_RENDERER
#define CONSTI10_FPV_VR_OS_VIDEO_RENDERER

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GLProgramTexture.h>
#include <GLProgramVC.h>
#include <GLBufferHelper.hpp>
#include <variant>
#include <TexturedGeometry.hpp>
#include <SphereBuilder.hpp>

class VideoRenderer{ //Does not inherit from IDrawable !
public:
    //Normal: Render a rectangle
    //Stereo: One decoded frame contains images for left and right eye
    //Render left frame into a rectangle with u->{0,0,5} and right frame int a rectangle with u->{0.5,1.0}
    //Degree360: 360 degree video, rendered onto a sphere instead of a quad
    enum VIDEO_RENDERING_MODE{
        RM_2D_MONOSCOPIC,RM_2D_STEREO,
        //2 different modes - mapping in shader or before uploading
        RM_360_DUAL_FISHEYE_INSTA360_1,
        RM_360_DUAL_FISHEYE_INSTA360_2,
        //Rest of the stuff added by webbn
        RM_360_KODAK_SP360_4K_DUAL,RM_360_KODAK_SP360_4K_SINGLE,RM_360_FIREFLY_SPLIT_4K,RM_360_1080P_USB,RM_360_STEREO_PI};
    /*
     * @param VIDEO_RENDERING_MODE one of the rendering modes above
     * @param videoTexture a valid OpenGL texture
     * @param DistortionManager: optional, used to create GLProgramTextureExt if needed
     */
    VideoRenderer(VIDEO_RENDERING_MODE mode,const GLuint videoTexture,const VDDCManager* vddcManager);
    void drawVideoCanvas(glm::mat4 ViewM, glm::mat4 ProjM, bool leftEye);
    void drawVideoCanvas360(glm::mat4 ViewM, glm::mat4 ProjM);
    //For 360 equirectangular we need the video vidth and height in px
    void updatePosition(const float positionZ,const float width,const float height,int optionalVideoWidthPx,int optionalVideoHeightPx);
    bool is360Video(){
        return mMode>1;
    }
public:
    VertexBuffer mEquirectangularSphereB; //Equirectangular Sphere
    VertexBuffer mInsta360SphereB;
    VertexIndexBuffer mVideoCanvasB;//whole video frame (u.v coordinates). Tesselated
    VertexIndexBuffer mVideoCanvasLeftEyeB;//left side of the video frame (u.v coordinates) Tesselated
    VertexIndexBuffer mVideoCanvasRightEyeB; //right side of the video frame (u.v coordinates) Tesselated

    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExt;
    std::unique_ptr<GLProgramTextureExt> mGLProgramTextureExtMappingEnabled;

    static const unsigned int TESSELATION_FACTOR=10;
    const VIDEO_RENDERING_MODE mMode;
    const GLuint mVideoTexture;
public:
    static TexturedGeometry::MeshY createMeshForMode(const VIDEO_RENDERING_MODE videoRenderingMode,const float positionZ, const float width, const float height){
        switch (videoRenderingMode){
            case RM_2D_MONOSCOPIC:
                return TexturedGeometry::MeshY(TexturedGeometry::makeTesselatedVideoCanvas(TESSELATION_FACTOR,{0,0,positionZ},{width,height},0.0f,1.0f),GL_TRIANGLES);
                break;
            case RM_2D_STEREO:
                return TexturedGeometry::MeshY(TexturedGeometry::makeTesselatedVideoCanvas(TESSELATION_FACTOR,{0,0,positionZ},{width,height},0.0f,1.0f),GL_TRIANGLES);
                break;
            case RM_360_DUAL_FISHEYE_INSTA360_1:
                return TexturedGeometry::MeshY(SphereBuilder::createSphereEquirectangularMonoscopic(),GL_TRIANGLE_STRIP);
                break;
            case RM_360_DUAL_FISHEYE_INSTA360_2:
                return TexturedGeometry::MeshY(SphereBuilder::createSphereDualFisheyeInsta360(),GL_TRIANGLE_STRIP);
                break;
            case RM_360_KODAK_SP360_4K_DUAL:
                return TexturedGeometry::MeshY(SphereBuilder::createSphereFisheye(UvSphere::ROTATE_0,0.5,0.65,190,0.05,0),GL_TRIANGLE_STRIP);
                break;
            case RM_360_KODAK_SP360_4K_SINGLE:
                return TexturedGeometry::MeshY(SphereBuilder::createSphereFisheye(UvSphere::ROTATE_180,0.5,0.5,190,0.0,0),GL_TRIANGLE_STRIP);
                break;
            case RM_360_FIREFLY_SPLIT_4K:
                return TexturedGeometry::MeshY(SphereBuilder::createSphereFisheye(UvSphere::ROTATE_180,0.5,0.5,210,0.05,0),GL_TRIANGLE_STRIP);
                break;
            case RM_360_1080P_USB:
                return TexturedGeometry::MeshY(SphereBuilder::createSphereFisheye(UvSphere::ROTATE_270,0.5,0.5,210,0.05,0),GL_TRIANGLE_STRIP);
                break;
            case RM_360_STEREO_PI:
                return TexturedGeometry::MeshY(SphereBuilder::createSphereFisheye(UvSphere::ROTATE_270,0.5,0.5,210,0.05,0),GL_TRIANGLE_STRIP);
                break;
            default:
                assert("Unknown type ");
        }
    }
};

#endif //CONSTI10_FPV_VR_OS_VIDEO_RENDERER
