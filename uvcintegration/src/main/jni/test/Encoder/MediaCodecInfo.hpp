//
// Created by geier on 25/05/2020.
//

#ifndef FPV_VR_OS_MEDIACODECINFO_HPP
#define FPV_VR_OS_MEDIACODECINFO_HPP

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
    // creates a purple rectangle with w=width/4 and h=height/2 that moves 1 square forward with frameIndex
    void generateFrame(int frameIndex, int colorFormat, uint8_t* frameData,size_t frameDataSize) {
        const int mWidth=640;
        const int mHeight=480;
        constexpr int TEST_Y = 120;                  // YUV values for colored rect
        constexpr int TEST_U = 160;
        constexpr int TEST_V = 200;

        const int HALF_WIDTH = mWidth / 2;
        boolean semiPlanar = isSemiPlanarYUV(colorFormat);
        // Set to zero.  In YUV this is a dull green.
        std::memset(frameData,0,frameDataSize);

        int startX, startY;
        frameIndex %= 8;
        //frameIndex = (frameIndex / 8) % 8;    // use this instead for debug -- easier to see
        if (frameIndex < 4) {
            startX = frameIndex * (mWidth / 4);
            startY = 0;
        } else {
            startX = (7 - frameIndex) * (mWidth / 4);
            startY = mHeight / 2;
        }
        for (int y = startY + (mHeight/2) - 1; y >= startY; --y) {
            for (int x = startX + (mWidth/4) - 1; x >= startX; --x) {
                if (semiPlanar) {
                    // full-size Y, followed by UV pairs at half resolution
                    // e.g. Nexus 4 OMX.qcom.video.encoder.avc COLOR_FormatYUV420SemiPlanar
                    // e.g. Galaxy Nexus OMX.TI.DUCATI1.VIDEO.H264E
                    //        OMX_TI_COLOR_FormatYUV420PackedSemiPlanar
                    frameData[y * mWidth + x] = (uint8_t) TEST_Y;
                    if ((x & 0x01) == 0 && (y & 0x01) == 0) {
                        frameData[mWidth*mHeight + y * HALF_WIDTH + x] = (uint8_t) TEST_U;
                        frameData[mWidth*mHeight + y * HALF_WIDTH + x + 1] = (uint8_t) TEST_V;
                    }
                } else {
                // full-size Y, followed by quarter-size U and quarter-size V
                // e.g. Nexus 10 OMX.Exynos.AVC.Encoder COLOR_FormatYUV420Planar
                // e.g. Nexus 7 OMX.Nvidia.h264.encoder COLOR_FormatYUV420Planar
                frameData[y * mWidth + x] = (uint8_t) TEST_Y;
                if ((x & 0x01) == 0 && (y & 0x01) == 0) {
                    frameData[mWidth*mHeight + (y/2) * HALF_WIDTH + (x/2)] = (uint8_t) TEST_U;
                    frameData[mWidth*mHeight + HALF_WIDTH * (mHeight / 2) +
                              (y/2) * HALF_WIDTH + (x/2)] = (uint8_t) TEST_V;
                }
            }
            }
        }
    }
}
#endif //FPV_VR_OS_MEDIACODECINFO_HPP
