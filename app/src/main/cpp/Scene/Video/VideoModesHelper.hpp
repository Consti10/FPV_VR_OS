//
// Created by geier on 23/06/2020.
//

#ifndef FPV_VR_OS_VIDEOMODESHELPER_HPP
#define FPV_VR_OS_VIDEOMODESHELPER_HPP

#include <TexturedGeometry.hpp>
#include <SphereBuilder.hpp>

class VideoModesHelper{
public:
    //Normal: Render a rectangle
    //Stereo: One decoded frame contains images for left and right eye
    //Render left frame into a rectangle with u->{0,0,5} and right frame int a rectangle with u->{0.5,1.0}
    //Degree360: 360 degree video, rendered onto a sphere instead of a quad
    enum VIDEO_RENDERING_MODE{
        RM_2D_MONOSCOPIC,
        RM_2D_STEREO,
        //2 different modes - mapping in shader or before uploading
        RM_360_DUAL_FISHEYE_INSTA360_1,
        RM_360_DUAL_FISHEYE_INSTA360_2,
        //Rest of the stuff added by webbn
        RM_360_KODAK_SP360_4K_DUAL,RM_360_KODAK_SP360_4K_SINGLE,RM_360_FIREFLY_SPLIT_4K,RM_360_1080P_USB,RM_360_STEREO_PI};

    static const unsigned int TESSELATION_FACTOR=10;
    static GLProgramTexture::TexturedMeshData createMeshForMode(const VIDEO_RENDERING_MODE videoRenderingMode,const float positionZ, const float width, const float height){
        switch (videoRenderingMode){
            case RM_2D_MONOSCOPIC:
                return TexturedGeometry::makeTesselatedVideoCanvas(TESSELATION_FACTOR,{0,0,positionZ},{width,height},0.0f,1.0f);
                break;
            case RM_2D_STEREO:
                return TexturedGeometry::makeTesselatedVideoCanvas(TESSELATION_FACTOR,{0,0,positionZ},{width,height},0.0f,1.0f);
                //break;
            case RM_360_DUAL_FISHEYE_INSTA360_1:
                return GLProgramTexture::convert(SphereBuilder::createSphereEquirectangularMonoscopic(),true);
                break;
            case RM_360_DUAL_FISHEYE_INSTA360_2:
                return SphereBuilder::createSphereDualFisheyeInsta360();
                break;
            case RM_360_KODAK_SP360_4K_DUAL:
                return SphereBuilder::createSphereFisheye(UvSphere::ROTATE_0,0.5,0.65,190,0.05,0);
                break;
            case RM_360_KODAK_SP360_4K_SINGLE:
                return SphereBuilder::createSphereFisheye(UvSphere::ROTATE_180,0.5,0.5,190,0.0,0);
                break;
            case RM_360_FIREFLY_SPLIT_4K:
                return SphereBuilder::createSphereFisheye(UvSphere::ROTATE_180,0.5,0.5,210,0.05,0);
                break;
            case RM_360_1080P_USB:
                return SphereBuilder::createSphereFisheye(UvSphere::ROTATE_270,0.5,0.5,210,0.05,0);
                break;
            case RM_360_STEREO_PI:
                return SphereBuilder::createSphereFisheye(UvSphere::ROTATE_270,0.5,0.5,210,0.05,0);
                break;
            default:
                assert("Unknown type ");
        }
    }
};
#endif //FPV_VR_OS_VIDEOMODESHELPER_HPP
