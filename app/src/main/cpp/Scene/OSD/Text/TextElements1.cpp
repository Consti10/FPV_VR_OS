#include <SettingsOSDStyle.h>
#include "TextElements1.h"

constexpr auto TAG="TextElements1";

TextElements1::TextElements1(const TextElements1::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,TelemetryReceiver& telemetryReceiver):
        IUpdateable(TAG),IDrawable(TAG),
        settingsOSDStyle(settingsOSDStyle),
        mTelemetryR(telemetryReceiver),
        mOptions(options),
        mGLTextObjIndices(allocateAllElements(settingsOSDStyle,options,batchingManager)) {
}


void TextElements1::setupPosition() {
    //Two rectangles filled with text. One on top and one on the bottom of the video.
    const auto rowsColumns= calculateRowsAndColumns(mWidth);
    const int nRowsPerColumn=rowsColumns[0];
    const int nUpperColumns=rowsColumns[1];
    const int nLowerColumns=rowsColumns[2];

    float heightUpper=nUpperColumns*mTextHeight;
    float heightLower=nLowerColumns*mTextHeight;
    float heightMiddle=mHeight-heightUpper-heightLower;
    IPositionable::Rect2D rectUpperText(mX,mY+heightLower+heightMiddle,mWidth,heightUpper);
    IPositionable::Rect2D rectMiddleVideo(mX,mY+heightLower,mWidth,heightMiddle);
    IPositionable::Rect2D rectLowerText(mX,mY,mWidth,heightLower);

    const float textObjW=mWidth/nRowsPerColumn;
    const float textObjH=mTextHeight;

    int column=0;
    int row=0;
    int idxVect=0;

    const auto size=mOptions.enableXX.size();
    for(size_t i=0;i<size;i++){
        column=idxVect/nRowsPerColumn;
        row=idxVect%nRowsPerColumn;
        //first,fill the upper rectangle with text
        if(column<nUpperColumns){
            //LOGD("1) Row %d Col %d Index %d",column,row,idxVect);
            auto& tmp=mGLTextObjIndices.at(i);
            tmp->setBounds(OSDTextObj::MIDDLE);
            tmp->setPosition(rectUpperText.mX + row * textObjW,
                             rectUpperText.mY + rectUpperText.mHeight - (column + 1) * textObjH,textObjW, textObjH);
            idxVect++;
        }
            //then, fill the lower rectangle with text
        else{
            column-=nUpperColumns;
            //LOGD("2) Row %d Col %d Index %d",column,row,idxVect);
            auto& tmp=mGLTextObjIndices.at(i);
            tmp->setBounds(OSDTextObj::MIDDLE);
            tmp->setPosition(rectLowerText.mX + row * textObjW,
                             rectLowerText.mY + rectLowerText.mHeight - (column + 1) * textObjH,
                              textObjW, textObjH);
            idxVect++;
        }
    }
}

void TextElements1::updateGL() {
    const unsigned int currStringToUpdate=getCyclicIndex(mOptions.enableXX.size()-1);
    auto& tmpTextObj= mGLTextObjIndices.at(currStringToUpdate);
    updateSubElement(currStringToUpdate,tmpTextObj.get());
}

void TextElements1::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {
    // nothing, everything is drawn by the Batching Manager
    //mPositionDebugUpper.drawGLDebug(ViewM,ProjM);
    //mPositionDebugMiddle.drawGLDebug(ViewM,ProjM);
    //mPositionDebugLower.drawGLDebug(ViewM,ProjM);
}

void TextElements1::updateSubElement(unsigned long whichSubStr,OSDTextObj* obj)const{
    MTelemetryValue tmp=mTelemetryR.getTelemetryValue(mOptions.enableXX.at(whichSubStr));
    if(tmp.hasIcon()){
        tmp.prefixIcon+=L" ";
    }else{
        tmp.prefix+=L": ";
    }
    tmp.metric=L" "+tmp.metric;
    const auto prefixColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR1;
    const auto valueColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR2;
    const auto metricsColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR3;
    std::vector<StylizedString> ss={{tmp.getPrefix(),tmp.prefixScale,prefixColor},{tmp.value,1,valueColor},{tmp.metric,0.83f,metricsColor}};
    obj->setTextSafe(ss);
    obj->recalculateDataIfNeeded();
}

glm::ivec3 TextElements1::calculateRowsAndColumns(float width) {
    const int nActiveElements=(int)mOptions.enableXX.size();
    const int nRowsPerColumnMax=(int)(width/GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE,mTextHeight));//calculate how many rows we can fit maximum into one column
    const auto nColumns=(int)std::ceil((float)nActiveElements/nRowsPerColumnMax);
    //now that we have the n of columns needed, we can try to order the elements more symmetrically
    //e.g if we need at least 2 columns, instead of putting 5 in the top and 3 in th bottom column
    //we place 4 elements per column.
    int nRowsPerColumn=nRowsPerColumnMax;
    if((nRowsPerColumnMax-1)*nColumns>=nActiveElements){
        nRowsPerColumn--;
    }
    const int nUpperColumns=nColumns-(nColumns/2);
    const int nLowerColumns=nColumns/2;
    return glm::ivec3(nRowsPerColumn,nUpperColumns,nLowerColumns);
}

//add on top/bottom of OSD overlay
IPositionable::Rect2D TextElements1::calculatePositionStereo(const IPositionable::Rect2D &osdOverlayCanvas) {
    auto rowCol= calculateRowsAndColumns(osdOverlayCanvas.mWidth);
    float textElementsHeightTop=mTextHeight*rowCol[1];
    float textElementsHeightBottom=mTextHeight*rowCol[2];
    //LOGD("W H %f %f",textElementsHeightTop,textElementsHeightBottom);
    const IPositionable::Rect2D ret(osdOverlayCanvas.mX,osdOverlayCanvas.mY-textElementsHeightBottom,
                                    osdOverlayCanvas.mWidth,osdOverlayCanvas.mHeight+textElementsHeightBottom+textElementsHeightTop);
    return ret;
}


std::vector<std::unique_ptr<OSDTextObj>>
TextElements1::allocateAllElements(const SettingsOSDStyle &settingsOSDStyle,const Options& options,BatchingManager &batchingManager) {
    std::vector<std::unique_ptr<OSDTextObj>> ret={};
    for(auto i:options.enableXX){
        ret.push_back(std::make_unique<OSDTextObj>(N_CHARS_PER_TEXT_OBJ,SettingsOSDStyle::isTransparentBackgroundEnabled(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH),
                                                   SettingsOSDStyle::getOSDBackgroundColor(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH), false,TrueColor2::WHITE,batchingManager));
    }
    return ret;
}




