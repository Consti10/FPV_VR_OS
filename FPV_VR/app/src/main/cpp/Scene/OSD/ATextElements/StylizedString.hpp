//
// Created by Consti10 on 14/05/2019.
//

#ifndef FPV_VR_PRIVATE_STYLIZEDSTRING_H
#define FPV_VR_PRIVATE_STYLIZEDSTRING_H

#include <stdlib.h>
#include <string.h>

//A Stylized String is a string with added scale and color
//Since OSD text might include different styles for sub-parts of one string (e.g. Dec: 60 fps) should have different colors for 'Dec' and '60fps'
//I usually use std::vector<StylizedString> in my code
class StylizedString{
public:
    std::wstring string;
    float scale;
    TrueColor color= Color::fromRGBA(1, 1, 1, 1);
    static bool equal(const std::vector<StylizedString>& text1,const std::vector<StylizedString>& text2){
        if(text1.size()!=text2.size())return false;
        for(unsigned int i=0;i<text1.size();i++){
            const auto& ss1=text1.at(i);
            const auto& ss2=text2.at(i);
            if(ss1.string!=ss2.string)return false;
            if(ss1.scale!=ss2.scale)return false;
            if(ss1.color!=ss2.color)return false;
        }
        return true;
    };
    static int length(const std::vector<StylizedString>& text1){
        int len=0;
        for(const auto& ss:text1){
            len+=ss.string.length();
        }
        return len;
    }
    static std::string debug(const std::vector<StylizedString>& text1){
        std::stringstream ss;
        for(const auto& tmp:text1){
            ss<<std::string(tmp.string.begin(),tmp.string.end());
        }
        return ss.str();
    }
};

#endif //FPV_VR_PRIVATE_STYLIZEDSTRING_H
