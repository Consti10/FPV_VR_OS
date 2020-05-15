//
// Created by Consti10 on 14/05/2019.
//

#ifndef FPV_VR_PRIVATE_STYLIZEDSTRING_H
#define FPV_VR_PRIVATE_STYLIZEDSTRING_H

#include <stdlib.h>
#include <string.h>
#include <TrueColor.hpp>
#include <codecvt>

//A Stylized String is a string with added scale and color
//Since OSD text might include different styles for sub-parts of one string (e.g. Dec: 60 fps) should have different colors for 'Dec' and '60fps'
//I usually use std::vector<StylizedString> in my code
class StylizedString{
public:
    std::wstring string=L"";
    float scale=1.0f;
    TrueColor color=TrueColor2::WHITE;
    StylizedString()= default;
    /*constexpr StylizedString(const std::wstring& string1=L"",float scale1=1.0f,TrueColor color1=TrueColor2::WHITE){
        string=string1;
        scale=scale1;
        color=color1;
    }*/
    bool operator==(const StylizedString& y)const{
        return (string.compare(y.string)==0) && scale==y.scale && color==y.color;
    }
    bool operator!=(const StylizedString& y)const{
        return !(*this==y);
    }
    static int length(const std::vector<StylizedString>& text1){
        int len=0;
        for(const auto& ss:text1){
            len+=ss.string.length();
        }
        return len;
    }
    std::string asNormalString()const{
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        const std::string converted_str = converter.to_bytes( ss.str());
        return converted_str;
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
