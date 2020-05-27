//
// Created by geier on 27/05/2020.
//

#ifndef FPV_VR_OS_MYCOLORSPACES_HPP
#define FPV_VR_OS_MYCOLORSPACES_HPP

#include <cstdint>

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

#endif //FPV_VR_OS_MYCOLORSPACES_HPP
