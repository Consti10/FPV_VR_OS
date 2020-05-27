//
// Created by geier on 25/05/2020.
//

#ifndef FPV_VR_OS_MEDIACODECINFO_HPP
#define FPV_VR_OS_MEDIACODECINFO_HPP

#include <array>

// Values taken from
// https://developer.android.com/reference/android/media/MediaCodecInfo.CodecCapabilities
namespace MediaCodecInfo{
    namespace CodecCapabilities{
        constexpr int COLOR_Format24bitRGB888=12;

        constexpr int COLOR_FormatYUV420Flexible=2135033992;
        constexpr int COLOR_FormatYUV420Planar=19;
        constexpr int COLOR_FormatYUV420PackedPlanar=20;
        constexpr int COLOR_FormatYUV420SemiPlanar=21;
        constexpr int COLOR_FormatYUV420PackedSemiPlanar=39;
        constexpr int COLOR_TI_FormatYUV420PackedSemiPlanar=2130706688;

        constexpr int COLOR_FormatYUV422Flexible= 2135042184;
        constexpr int COLOR_FormatYUV422Planar=22;
        constexpr int COLOR_FormatYUV422PackedPlanar=23;
        constexpr int COLOR_FormatYUV422SemiPlanar=24;
        constexpr int COLOR_FormatYUV422PackedSemiPlanar=40;

        constexpr int COLOR_FormatYUV444Flexible=2135181448;
    }
};

namespace MyColorSpaces{
    // Y plane has full width & height
    // U and V plane both have half width and full height
    template<size_t WIDTH,size_t HEIGHT>
    class YUV422Planar{
    public:
        uint8_t planeY[WIDTH][HEIGHT];
        uint8_t planeU[WIDTH/2][HEIGHT];
        uint8_t planeV[WIDTH/2][HEIGHT];
    }__attribute__((packed));
    //
    template<size_t WIDTH,size_t HEIGHT>
    class YUV420SemiPlanar{
    public:
        uint8_t planeY[HEIGHT][WIDTH];
        uint8_t planeUV[HEIGHT/2][WIDTH/2][2];
    }__attribute__((packed));
    static_assert(sizeof(YUV420SemiPlanar<640,480>)==640*480*12/8);
}

// taken from https://android.googlesource.com/platform/cts/+/3661c33/tests/tests/media/src/android/media/cts/EncodeDecodeTest.java
// and translated to cpp
namespace YUVFrameGenerator{
    boolean isSemiPlanarYUV(const int colorFormat) {
        using namespace MediaCodecInfo::CodecCapabilities;
        switch (colorFormat) {
            case COLOR_FormatYUV420Planar:
            case COLOR_FormatYUV420PackedPlanar:
                return false;
            case COLOR_FormatYUV420SemiPlanar:
            case COLOR_FormatYUV420PackedSemiPlanar:
            case COLOR_TI_FormatYUV420PackedSemiPlanar:
                return true;
            default:
                MLOGE<<"unknown format "<< colorFormat;
                return true;
        }
    }
    // YUV values for purple
    constexpr uint8_t PURPLE_Y = 120;
    constexpr uint8_t PURPLE_U = 160;
    constexpr uint8_t PURPLE_V = 200;

    // creates a purple rectangle with w=width/4 and h=height/2 that moves 1 square forward with frameIndex
    void generateFrame(int frameIndex, int colorFormat, uint8_t* frameData,size_t frameDataSize) {
        using namespace MyColorSpaces;
        // Full width/height for luma ( Y )
        constexpr size_t WIDTH=640;
        constexpr size_t HEIGHT=480;
        constexpr size_t FRAME_BUFFER_SIZE_B= WIDTH * HEIGHT * 12 / 8;
        if(frameDataSize < FRAME_BUFFER_SIZE_B){
            MLOGE<<"Frame buffer size not suefficcient";
            return;
        }
        // Half width / height for chroma (U,V btw Cb,Cr)
        constexpr size_t HALF_WIDTH= WIDTH / 2;
        constexpr size_t HALF_HEIGHT= HEIGHT / 2;

        auto& framebuffer= *static_cast<YUV420SemiPlanar<640,480>*>(static_cast<void*>(frameData));

        boolean semiPlanar = isSemiPlanarYUV(colorFormat);
        // Set to zero.  In YUV this is a dull green.
        std::memset(frameData, 0, FRAME_BUFFER_SIZE_B);

        constexpr int COLORED_RECT_W= WIDTH / 4;
        constexpr int COLORED_RECT_H= HEIGHT / 2;

        //frameIndex %= 8;
        frameIndex = (frameIndex / 8) % 8;    // use this instead for debug -- easier to see
        int startX;
        const int startY=frameIndex<4 ? 0 : HEIGHT / 2;
        if (frameIndex < 4) {
            startX = frameIndex * COLORED_RECT_W;
        } else {
            startX = frameIndex % 4 * COLORED_RECT_W;
        }
        // fill the wanted area with purple color
        for (int x = startX; x < startX + COLORED_RECT_W; x++) {
            for (int y = startY; y < startY + COLORED_RECT_H; y++) {
                if (semiPlanar) {
                    // full-size Y, followed by UV pairs at half resolution
                    // e.g. Nexus 4 OMX.qcom.video.encoder.avc COLOR_FormatYUV420SemiPlanar
                    // e.g. Galaxy Nexus OMX.TI.DUCATI1.VIDEO.H264E
                    //        OMX_TI_COLOR_FormatYUV420PackedSemiPlanar
                    //frameData[y * WIDTH + x] = (uint8_t) TEST_Y;
                    framebuffer.planeY[y][x]=PURPLE_Y;
                    const bool even=(x % 2) == 0 && (y % 2) == 0;
                    if (even) {
                        framebuffer.planeUV[y/2][x/2][0]=PURPLE_U;
                        framebuffer.planeUV[y/2][x/2][1]=PURPLE_V;
                        //frameData[WIDTH * HEIGHT + y * HALF_WIDTH + x] = TEST_U;
                        //frameData[WIDTH * HEIGHT + y * HALF_WIDTH + x + 1] = TEST_V;
                    }
                } else {
                    // full-size Y, followed by quarter-size U and quarter-size V
                    // e.g. Nexus 10 OMX.Exynos.AVC.Encoder COLOR_FormatYUV420Planar
                    // e.g. Nexus 7 OMX.Nvidia.h264.encoder COLOR_FormatYUV420Planar
                    // NOT TESTED !
                    frameData[y * WIDTH + x] = PURPLE_Y;
                    if ((x & 0x01) == 0 && (y & 0x01) == 0) {
                        frameData[WIDTH * HEIGHT + (y / 2) * HALF_WIDTH + (x / 2)] = PURPLE_U;
                        frameData[WIDTH * HEIGHT + HALF_WIDTH * (HEIGHT / 2) +
                                  (y / 2) * HALF_WIDTH + (x / 2)] = PURPLE_V;
                    }
                }
            }
        }
    }


    // For some reason HEIGHT comes before WIDTH here ?!
    // The Y plane has full resolution.
    //auto& YPlane = *static_cast<uint8_t (*)[HEIGHT][WIDTH]>(static_cast<void*>(frameData));
    // The CbCrPlane only has half resolution in both x and y direction ( 4:2:0 )
    // CbCrPlane[y][x][0] == Cb (U) value for pixel x,y and
    // CbCrPlane[y][x][1] == Cr (V) value for pixel x,y
    //auto& CbCrPlane = *static_cast<uint8_t(*)[HALF_HEIGHT][HALF_WIDTH][2]>(static_cast<void*>(&frameData[WIDTH * HEIGHT]));
    // Check - YUV420 has 12 bit per pixel (1.5 bytes)
    //static_assert(sizeof(YPlane)+sizeof(CbCrPlane) == FRAME_BUFFER_SIZE_B);
}
#endif //FPV_VR_OS_MEDIACODECINFO_HPP
