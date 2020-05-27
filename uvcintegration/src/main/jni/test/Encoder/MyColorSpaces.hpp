//
// Created by geier on 27/05/2020.
//

#ifndef FPV_VR_OS_MYCOLORSPACES_HPP
#define FPV_VR_OS_MYCOLORSPACES_HPP

#include <cstdint>
#include <array>

// Watch out: For the data layout height comes before width !!
// I like to use it the other way around - image data = [width][height]
namespace MyColorSpaces{
    // Y plane has full width & height
    // U and V plane both have half width and full height
    template<size_t WIDTH,size_t HEIGHT>
    class YUV422Planar{
    public:
        uint8_t planeY[HEIGHT][WIDTH];
        uint8_t planeU[HEIGHT][WIDTH/2];
        uint8_t planeV[HEIGHT][WIDTH/2];
        std::array<uint8_t,2> getUVHalf(size_t xHalf,size_t yHalf){
            return {planeU[yHalf*2][xHalf],planeV[yHalf*2][xHalf]};
        }
    }__attribute__((packed));
    template<size_t WIDTH,size_t HEIGHT>
    class YUV420SemiPlanar{
    public:
        uint8_t planeY[HEIGHT][WIDTH];
        uint8_t planeUV[HEIGHT/2][WIDTH/2][2];
        void setY(size_t x, size_t y, uint8_t value){
            planeY[y][x]=value;
        }
        void setUV(size_t x, size_t y, uint8_t valueU, uint8_t valueV){
            planeUV[y][x][0]=valueU;
            planeUV[y][x][1]=valueV;
        }
    }__attribute__((packed));
    static_assert(sizeof(YUV420SemiPlanar<640,480>)==640*480*12/8);

    //void convert123(const YUV422Planar<640,480>& in,YUV420SemiPlanar<640,480>& out){
    //
    //}
}

#endif //FPV_VR_OS_MYCOLORSPACES_HPP
