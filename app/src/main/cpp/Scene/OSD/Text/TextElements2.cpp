//
// Created by Constantin on 6/10/2018.
//
#include <SettingsOSDStyle.h>
#include "TextElements2.h"
#include <GLHelper.hpp>

constexpr auto TAG="TextElements2";

TextElements2::TextElements2(const TextElements2::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,TelemetryReceiver& telemetryReceiver):
        IUpdateable(TAG),IDrawable(TAG),
        settingsOSDStyle(settingsOSDStyle),
        mTelemetryReceiver(telemetryReceiver),
        mOptions(options),
        mGLTextObjIndices(createAllElements(settingsOSDStyle,batchingManager,options)){
}

void TextElements2::setupPosition() {
    const float bigRowHeight=mTextHeight*TEXT_UPSCALE_FACTOR;
    const float smallRowHeight=mTextHeight*TEXT_DOWNSCALE_FACTOR;

    const auto size=mOptions.enableXX.size();
    for(unsigned int i=0;i<size;i++){
        auto tmp=mGLTextObjIndices.at(i).get();
        const auto ID=mOptions.enableXX.at(i);
        switch(ID){
            //Upper right corner
            case TelemetryReceiver::EZWB_UPLINK_RC_RSSI:{
                const float maxLength1=GLProgramText::getStringLength(L"-99dBmX",bigRowHeight);
                tmp->setPosition(mX + mWidth - maxLength1,
                                 mY + mHeight - bigRowHeight, maxLength1, bigRowHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::EZWB_UPLINK_RC_BLOCKS:{
                const float maxLength2=GLProgramText::getStringLength(L"1024/1024",mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength2,
                                 mY + mHeight - bigRowHeight-mTextHeight, maxLength2, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::EZWB_STATUS_AIR:{
                const float offset=(bigRowHeight+mTextHeight);
                const float maxLength3=GLProgramText::getStringLength(L"Gnd 100% 999*",mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength3,
                                 mY + mHeight - (offset+mTextHeight), maxLength3, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::EZWB_STATUS_GROUND:{
                const float offset=(bigRowHeight+mTextHeight);
                const float maxLength3=GLProgramText::getStringLength(L"Gnd 100% 999*",mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength3,
                                 mY + mHeight - (offset+mTextHeight*2), maxLength3, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            //lower right corner
            case TelemetryReceiver::VS:{
                float maxLength2=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_2_1,mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength2,
                                 mY + mTextHeight * 3, maxLength2, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::HS_GROUND:{
                float maxLength2=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_2_1,mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength2,
                                 mY + mTextHeight * 2, maxLength2, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::LATITUDE:{
                float maxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_2,mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength,
                                 mY + mTextHeight * 1, maxLength, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::LONGITUDE:{
                float maxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_2,mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength,
                                 mY + mTextHeight * 0, maxLength, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::HOME_DISTANCE:{
                float maxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_2,mTextHeight);
                float maxLength2=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_2_1,mTextHeight);
                tmp->setPosition(mX + mWidth - maxLength - maxLength2,
                                 mY + mTextHeight * 0, maxLength2, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            //lower middle
            //Use bounds right for the sub elements even though that they are on the left side
            case TelemetryReceiver::FLIGHT_STATUS_MAV_ONLY:{
                float maxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_FLIGHT_MODE,mTextHeight);
                tmp->setPosition(mX + mWidth/2.0f - maxLength/2.0f,
                                 mY + mTextHeight * 1, maxLength, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            //lower left corner
            case TelemetryReceiver::SATS_IN_USE:{
                float maxLength=GLProgramText::getStringLength(L"100 %",mTextHeight);
                tmp->setPosition(mX, mY + mTextHeight * 1, maxLength, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::BATT_PERCENTAGE:{
                float maxLength=GLProgramText::getStringLength(L"100 %",mTextHeight);
                tmp->setPosition(mX, mY + mTextHeight * 0, maxLength, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::BATT_VOLTAGE:{
                float maxLength=GLProgramText::getStringLength(L"99.99 V",mTextHeight);
                tmp->setPosition(mX + maxLength, mY + mTextHeight * 1, maxLength, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            case TelemetryReceiver::BATT_CURRENT:{
                float maxLength=GLProgramText::getStringLength(L"99.99 V",mTextHeight);
                tmp->setPosition(mX + maxLength, mY + mTextHeight * 0, maxLength, mTextHeight);
                tmp->setBounds(OSDTextObj::RIGHT);
            }break;
            //upper left corner
            case TelemetryReceiver::EZWB_DOWNLINK_VIDEO_RSSI:{
                float te1MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_1,bigRowHeight);
                tmp->setPosition(mX, mY + mHeight - bigRowHeight, te1MaxLength, bigRowHeight);
            }break;
            case TelemetryReceiver::EZWB_DOWNLINK_VIDEO_RSSI2:{
                float te1MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_1,bigRowHeight);
                float te2MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_2,bigRowHeight);
                tmp->setPosition(mX + te1MaxLength, mY + mHeight - bigRowHeight, te2MaxLength,
                                 bigRowHeight);
            }break;
            case TelemetryReceiver::EZWB_BLOCKS:{
                float te3MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_3,smallRowHeight);
                tmp->setPosition(mX, mY + mHeight - bigRowHeight - mTextHeight, te3MaxLength,
                                 mTextHeight);
            }break;
            case TelemetryReceiver::EZWB_RSSI_ADAPTER0:{
                float te3MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_3,smallRowHeight);
                float startY=mY+mHeight-bigRowHeight-mTextHeight-smallRowHeight;
                tmp->setPosition(mX, startY - 0 * smallRowHeight, te3MaxLength,
                                 smallRowHeight);
            }break;
            case TelemetryReceiver::EZWB_RSSI_ADAPTER1:{
                float te3MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_3,smallRowHeight);
                float startY=mY+mHeight-bigRowHeight-mTextHeight-smallRowHeight;
                tmp->setPosition(mX, startY - 1 * smallRowHeight, te3MaxLength,
                                 smallRowHeight);
            }break;
            case TelemetryReceiver::EZWB_RSSI_ADAPTER2:{
                float te3MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_3,smallRowHeight);
                float startY=mY+mHeight-bigRowHeight-mTextHeight-smallRowHeight;
                tmp->setPosition(mX, startY - 2 * smallRowHeight, te3MaxLength,
                                 smallRowHeight);
            }break;
            case TelemetryReceiver::EZWB_RSSI_ADAPTER3:{
                float te3MaxLength=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE_EZWB_3,smallRowHeight);
                float startY=mY+mHeight-bigRowHeight-mTextHeight-smallRowHeight;
                tmp->setPosition(mX, startY - 3 * smallRowHeight, te3MaxLength,
                                 smallRowHeight);
            }break;
            default:
                MLOGE<<"Should not happen "<<ID;
                break;
        }
    }
}

void TextElements2::updateGL() {
    const unsigned int currStringToUpdate=getCyclicIndex(mOptions.enableXX.size()-1);
    auto tmpTextObj= mGLTextObjIndices.at(currStringToUpdate).get();
    updateSubElement((int) currStringToUpdate,tmpTextObj);
    //MLOGD<<"Batt "<<mTelemetryReceiver.uav_td.BatteryPack_V<<" "<<mTelemetryReceiver.uav_td.BatteryPack_A<<" "<<mTelemetryReceiver.uav_td.BatteryPack_mAh;
}

void TextElements2::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {

}

void TextElements2::updateSubElement(unsigned long id,OSDTextObj* obj)const {
    if(!mOptions.enableXX.at(id)){
        return;
    }
    const auto prefixColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR1;
    const auto valueColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR2;
    const auto metricsColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR3;
    const auto iconColor=prefixColor;

    const auto ID=mOptions.enableXX.at(id);
    MTelemetryValue tmp=mTelemetryReceiver.getTelemetryValue(ID);
    if(tmp.prefix.empty()){
    }else{
        tmp.prefix+=L" ";
    }
    std::vector<StylizedString> ss;
    if(ID==TelemetryReceiver::BATT_VOLTAGE || ID==TelemetryReceiver::BATT_PERCENTAGE || ID==TelemetryReceiver::BATT_CURRENT){
        ss={{tmp.value,1,valueColor},{L" "+tmp.metric,1.0f,metricsColor}};
    }else if(ID==TelemetryReceiver::SATS_IN_USE) {
        ss = {{L" " + tmp.getPrefix(), 1.0f, prefixColor},
              {tmp.value,         1,    valueColor}};
    }else if(ID==TelemetryReceiver::EZWB_DOWNLINK_VIDEO_RSSI || ID==TelemetryReceiver::EZWB_UPLINK_RC_RSSI) {
        ss = {{tmp.value,  1,    valueColor},
              {tmp.metric, 1.0f, metricsColor}};
    }else{
        ss={{tmp.getPrefix(),tmp.prefixScale,prefixColor},{tmp.value,1,valueColor},{tmp.metric,1.0f,metricsColor}};
    }
    //else if(ID==TelemetryReceiver::EZWB_RSSI_ADAPTER0 ||ID==TelemetryReceiver::EZWB_RSSI_ADAPTER1 ||ID==TelemetryReceiver::EZWB_RSSI_ADAPTER2 ||ID==TelemetryReceiver::EZWB_RSSI_ADAPTER3){}
    obj->setTextSafe(ss);
    obj->recalculateDataIfNeeded();
}

std::vector<std::unique_ptr<OSDTextObj>>
TextElements2::createAllElements(const SettingsOSDStyle &settingsOSDStyle,
                                 BatchingManager &batchingManager,const TextElements2::Options& options) {
    std::vector<std::unique_ptr<OSDTextObj>> ret={};
    const int N_CHARS_PER_TEXT_OBJ=20;
    for(auto i:options.enableXX){
        ret.push_back(std::make_unique<OSDTextObj>(N_CHARS_PER_TEXT_OBJ,SettingsOSDStyle::isTransparentBackgroundEnabled(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH),
                                                   SettingsOSDStyle::getOSDBackgroundColor(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH), false,TrueColor2::WHITE,batchingManager));
    }
    return ret;
}

/*std::vector<std::unique_ptr<OSDTextObj>
TextElements2::createAllElements(const SettingsOSDStyle& settingsOSDStyle,BatchingManager &batchingManager) {
    std::array<unsigned int,N_TEXT_OBJ> sizes={15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,20,20,20,20,20,20};
    std::array<OSDTextObj*,N_TEXT_OBJ> ret={};
    for(unsigned int i=0;i<N_TEXT_OBJ;i++){
        auto* tmp=new OSDTextObj(sizes.at(i),SettingsOSDStyle::isTransparentBackgroundEnabled(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH),
                                       SettingsOSDStyle::getOSDBackgroundColor(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH), false,
                                 Color::fromRGBA(1, 1, 1, 1),batchingManager);
        ret.at(i)=tmp;
    }
    return ret;
}*/

