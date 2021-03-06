//
// Created by Constantin on 8/25/2018.
//

#ifndef FPV_VR_OSDBACKGROUNDOBJ_H
#define FPV_VR_OSDBACKGROUNDOBJ_H

#include <ColoredGeometry.hpp>
#include "General/IPositionable.hpp"
#include "OSD/ElementBatching/BatchingManager.h"

/**
 * Uses BatchingManager and therefore can be used by OSD elements with little overhead
 * Can be used to draw a semi-transparent rectangular background over the video to make OSD elements appear more clearly
 */

class OSDBackgroundObject {
private:
    const TrueColor mBackgroundColor;
    IPositionable::Rect2D_ mPosition;
    static constexpr const int N_BACKGROUND_VERTICES=3*4;
    std::shared_ptr<ModifiableArray<ColoredVertex>> backgroundBuffer;
public:
    OSDBackgroundObject(BatchingManager& batchingManager,const TrueColor backgroundColor):
            mBackgroundColor(backgroundColor){
        backgroundBuffer=batchingManager.allocateVCTriangles(N_BACKGROUND_VERTICES);
    }
    void setPosition(float x, float y,float w, float h){
        mPosition.X=x;
        mPosition.Y=y;
        mPosition.Width=w;
        mPosition.Height=h;
    }
    void recalculateData(){
        ColoredGeometry::makeBackgroundRect(backgroundBuffer->modify(),glm::vec3(mPosition.X,mPosition.Y,0),
                           mPosition.Width,mPosition.Height,mBackgroundColor,mBackgroundColor);
    }
};

#endif //FPV_VR_OSDBACKGROUNDOBJ_H
