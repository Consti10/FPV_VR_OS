
#ifndef OSDTEXTOBJ
#define OSDTEXTOBJ


#include <Helper/ColoredGeometry.hpp>
#include <Helper/MDebug.hpp>
#include "StylizedString.hpp"
#include "../ElementBatching/CpuGpuBuff.h"
#include "../../General/IPositionable.hpp"
#include "../ElementBatching/BatchingManager.h"
#include <GLProgramText.h>

/**
 * Uses BatchingManager and therefore can be used by OSD elements with little overhead
 */



class OSDTextObj {
public:
    static constexpr const int N_BACKGROUND_VERTICES=3*4;
    static constexpr const int N_OUTLINE_VERTICES=4*2;
    inline static const std::string TAG="OSDTextObj";
    enum BOUNDS{LEFT,MIDDLE,RIGHT};
    const bool enableOutline;
    const bool enableBackground;
    const int maxNChars;
    ModifiableArray<GLProgramText::Character>* textBuffer;
    ModifiableArray<GLProgramVC::Vertex>* backgroundBuffer;
    ModifiableArray<GLProgramVC::Vertex>* outlineBuffer;
private:
    const TrueColor mBackgroundColor;
    const TrueColor mOutlineColor;
    BOUNDS mBounds=LEFT;
    IPositionable::Rect2D_ mPosition;
    int nActiveChars=0;
    std::vector<StylizedString> mText;
    bool outlineRecalculationNeeded=true; //The outline does only change when the position is changed
    bool textRecalculationNeeded=true; //the text changes when the content of mText change
    bool backgroundRecalculationNeeded=true; //same for the background, but it does not change when the text color changes
public:
    OSDTextObj(int maxNChars,bool eBackground,TrueColor backgroundColor,bool eOutline,TrueColor outlineColor,BatchingManager& batchingManager):
            maxNChars(maxNChars),
            enableBackground(eBackground),
            mBackgroundColor(backgroundColor),
            enableOutline(eOutline),
            mOutlineColor(outlineColor){
        textBuffer=batchingManager.allocateText(maxNChars);
        if(enableBackground){
            backgroundBuffer=batchingManager.allocateVCTriangles(N_BACKGROUND_VERTICES);
        }
        if(enableOutline){
            outlineBuffer= batchingManager.allocateVCLines(N_OUTLINE_VERTICES);
        }
        setPosition(0,0,0,0,0);
        setTextSafe(L"INIT");
        recalculateDataIfNeeded();
    }

    void setBounds(const BOUNDS bounds){
        mBounds=bounds;
        textRecalculationNeeded=true;
        backgroundRecalculationNeeded=true;
    }

    void setPosition(float x, float y, float z, float w, float h){
        mPosition.X=x;
        mPosition.Y=y;
        mPosition.Z=z;
        mPosition.Width=w;
        mPosition.Height=h;
        outlineRecalculationNeeded=true;
        textRecalculationNeeded=true;
        backgroundRecalculationNeeded=true;
    }
    void setTextSafe(const std::wstring& text){
        const std::vector<StylizedString> ss={{text,1.0f,Color::fromRGBA(1,1,1,1)}};
        setTextSafe(ss);
    }
    void setTextSafe(const std::wstring &text,const TrueColor textColor){
        std::vector<StylizedString> ss={{text,1.0f,textColor}};
        setTextSafe(ss);
    }
    void setTextSafe(const std::vector<StylizedString>& ss){
        if(StylizedString::equal(mText,ss)){
            return;
        }
        if(StylizedString::length(ss)>maxNChars){
            MDebug::log(StylizedString::debug(ss),TAG);
            mText={{L"E>n",1.0f,Color::fromRGBA(1, 1, 1, 1)}};
        }else{
            mText=ss;
        }
        textRecalculationNeeded=true;
        backgroundRecalculationNeeded=true;
    }

    void recalculateDataIfNeeded(){
        float currTextW=getStringLength(mText,mPosition.Height);
        float xOffset=0;
        switch(mBounds){
            case LEFT:
                break;
            case MIDDLE:
                xOffset=mPosition.Width/2.0f-currTextW/2.0f;
                break;
            case RIGHT:
                xOffset=mPosition.Width-currTextW;
                break;
            default:
                break;
        }
        if(xOffset<0 || xOffset>mPosition.Width){
            xOffset=0;
            __android_log_print(ANDROID_LOG_DEBUG,TAG.c_str(),"ERROR: xOff too big/small %ls xOffset: %f",mText.at(0).string.c_str(),xOffset);
        }
        if(textRecalculationNeeded){
            textBuffer->zeroContent();
            const auto lastNActiveChars=nActiveChars;
            nActiveChars=0;
            float xAdvance=0;
            for (const auto &text : mText) {
                const float yOffset=(1- text.scale)*mPosition.Height*0.5f;
                const float scale= text.scale*mPosition.Height;
                const auto& string= text.string;
                const auto color= text.color;
                //__android_log_print(ANDROID_LOG_DEBUG,TAG.c_str(),"%d:%s %d",i,string.c_str(),scale);
                nActiveChars+= GLProgramText::convertStringToRenderingData(
                        mPosition.X + xOffset +
                        xAdvance,
                        mPosition.Y + yOffset,
                        mPosition.Z, scale, string,
                        color,
                        textBuffer->modify(),
                        nActiveChars);
                xAdvance+=GLProgramText::getStringLength(string,scale);
            }
            const unsigned int nModifiedElements=(unsigned int)std::max(lastNActiveChars,nActiveChars);
            //__android_log_print(ANDROID_LOG_DEBUG,TAG.c_str(),"chars %d %d",lastNActiveChars,nActiveChars);
            textBuffer->modify(nModifiedElements);
            textRecalculationNeeded=false;
        }
        if(enableBackground && backgroundRecalculationNeeded){
            ColoredGeometry::makeBackgroundRect(backgroundBuffer->modify(),glm::vec3(mPosition.X+xOffset,mPosition.Y,mPosition.Z),
                               currTextW,mPosition.Height,mBackgroundColor,mBackgroundColor);
            backgroundRecalculationNeeded=false;
        }
        if(enableOutline && outlineRecalculationNeeded){
            ColoredGeometry::makeOutlineQuadWithLines(outlineBuffer->modify(),mPosition.X,mPosition.Y,mPosition.Z,mPosition.Width,mPosition.Height,
                                     mOutlineColor);
            outlineRecalculationNeeded=false;
        }
    }


    static float getStringLength(const std::vector<StylizedString>& text,float height){
        float width=0;
        for(const StylizedString& ss: text){
            width+=GLProgramText::getStringLength(ss.string,height*ss.scale);
        }
        return width;
    }

    static float calculateBiggestFittingScale(const std::wstring& text,float height,float width){
        float textWidthScale1=GLProgramText::getStringLength(text,height);
        if(textWidthScale1<=width)return textWidthScale1;
        float scale=width/textWidthScale1;
        return scale;
    }

    static std::vector<std::unique_ptr<OSDTextObj>> createAll(const int maxNChars,const int nOfGLTextObjects,const bool eBackground,
                                              const TrueColor backgroundColor,const bool eOutline,const TrueColor outlineColor,BatchingManager& batchingManager){
        std::vector<std::unique_ptr<OSDTextObj>> ret={};
        for(int i=0;i<nOfGLTextObjects;i++){
            auto tmp=std::make_unique<OSDTextObj>(maxNChars,eBackground,backgroundColor,eOutline,outlineColor,batchingManager);
            ret.push_back(std::move(tmp));
        }
        return ret;
    }

    /*static std::vector<std::unique_ptr<OSDTextObj>> createAll(const int maxNChars,const int nOfGLTextObjects,const bool eBackground,
                                              const TrueColor backgroundColor,const bool eOutline,const TrueColor outlineColor,BatchingManager& batchingManager){
        std::vector<std::unique_ptr<OSDTextObj>> ret={};
        for(int i=0;i<nOfGLTextObjects;i++){
            auto tmp=std::make_unique<OSDTextObj>(maxNChars,eBackground,backgroundColor,eOutline,outlineColor,batchingManager);
            ret.push_back(std::move(tmp));
        }
        return ret;
    }*/
};

#endif //OSDTEXTOBJ
