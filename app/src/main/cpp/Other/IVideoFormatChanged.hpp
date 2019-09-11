//
// Created by Constantin on 1/22/2019.
//

#ifndef FPV_VR_IVIDEOFORMATCHANGED_H
#define FPV_VR_IVIDEOFORMATCHANGED_H

class IVideoFormatChanged{
protected:
    float lastVideoFormat=1.77777f;
    std::atomic_bool videoFormatChanged{false};
public:
    IVideoFormatChanged()= default;
    /**
     * OPTIONAL: notify the scene that the ratio of the FPV video stream has changed
     * The OpenGL renderer might start before the first video frame is available (initialized by default with 1280x720)
     * If the video ratio changes, a recalculation of the scene object positions is required
     * @param videoW Video width, in pixels
     * @param videoH Video height, in pixels
     * The recalculation happens in onDrawFrame, since it needs a valid OpenGL context
     * But this function may be called from an non-OpenGL thread
     */
    void SetVideoRatio(int videoW,int videoH){
        lastVideoFormat=((float)videoW)/(float)videoH;
        videoFormatChanged=true;
    }
private:
};

#endif //FPV_VR_IVIDEOFORMATCHANGED_H
