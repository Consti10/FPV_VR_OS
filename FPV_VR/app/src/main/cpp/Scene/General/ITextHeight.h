//
// Created by Constantin on 8/20/2018.
//

#ifndef FPV_VR_ITEXTHEIGHT_H
#define FPV_VR_ITEXTHEIGHT_H

class ITextHeight{
public:
    void setTextHeight(float textH){
        mTextHeight=textH;
    }
protected:
    float mTextHeight=1;
};

#endif //FPV_VR_ITEXTHEIGHT_H
