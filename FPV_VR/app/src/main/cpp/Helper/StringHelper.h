//
// Created by Constantin on 09.10.2017.
//

#ifndef OSDTESTER_STRINGHELPER_H
#define OSDTESTER_STRINGHELPER_H

#include <string>
#include <sstream>
#include "android/log.h"

#define TAG3 "StringHelper"
#define LOGV3(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG3, __VA_ARGS__)
using namespace std;

static const string intToString(const int value,const int maxStringL){
    stringstream s;
    s << value;
    if(s.str().length()>maxStringL){
        return "E";
    }
    return s.str();
}
static const string floatToString(const float value,const int max_length,const int res_afterKomma){
    int beforeCome=(int)value;
    float afterCome=value-((int)value);
    //LOGV3("Number:%f, beforeCome:%d afterCome:%f",value,beforeCome,afterCome);
    stringstream tmp;
    tmp<<beforeCome;

    int stringLengthBeforeCome=(int)tmp.str().length();
    if(stringLengthBeforeCome>max_length){
        return "E";
    }
    if(stringLengthBeforeCome+1>=max_length || res_afterKomma==0){
        //e.g. return 10 instead of 10.
        return tmp.str();
    }
    tmp<<".";
    int t=(int)(afterCome*(max_length-stringLengthBeforeCome-1)*10.0f);
    tmp<<t;
    /*for(int i=0;(i<max_length-(stringLengthBeforeCome+1)) && (i<res_afterKomma);i++){
        int t=(int)(afterCome*10.0f);
        tmp<<t;
        afterCome-=((float)t/10.0f);
        afterCome*=10;
    }*/
    if(tmp.str().length()>max_length){
        return "E2";
    }
    return tmp.str();
}










#endif //OSDTESTER_STRINGHELPER_H
