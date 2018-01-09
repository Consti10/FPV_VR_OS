
#include "GLRenderText.h"
#include "../Helper/GLHelper.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <vector>

#define TAG "GLRenderText"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

constexpr int CELL_WIDTH_HEIGHT_PIXELS=64;
constexpr int IMAGE_WIDTH_HEIGHT_PIXELS=1024;

constexpr float CELL_WIDTH_HEIGHT_UV=1.0f/((float)IMAGE_WIDTH_HEIGHT_PIXELS/(float)CELL_WIDTH_HEIGHT_PIXELS);

constexpr int FONT_WIDTH_PIXELS[] = {41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,
                           15,17,20,31,31,49,37,11,18,18,21,32,15,18,15,15,31,31,31,31,31,31,31,31,31,31,15,15,32,32,32,31,
                           56,37,37,40,40,37,34,43,40,15,28,37,31,45,40,43,37,43,40,37,33,40,37,54,35,35,34,15,15,15,24,31,
                           18,31,31,28,31,31,15,31,31,11,13,28,11,47,31,31,31,31,18,28,15,31,29,39,27,27,27,18,14,18,32,41,
                           31,41,12,31,18,55,31,31,18,56,37,18,55,41,34,41,41,12,12,18,18,19,31,55,16,55,28,18,52,41,27,37,
                           15,17,31,31,31,31,14,31,18,41,20,31,32,18,41,30,22,30,18,18,18,32,30,18,18,18,20,31,46,46,46,34,
                           37,37,37,37,37,37,55,40,37,37,37,37,15,15,15,15,40,40,43,43,43,43,43,32,43,40,40,40,40,37,37,34,
                           31,31,31,31,31,31,49,28,31,31,31,31,15,15,15,15,31,31,31,31,31,31,31,30,34,31,31,31,31,28,31,28
};


GLRenderText::GLRenderText(const bool distCorrection) {
    distortionCorrection=distCorrection;
    if(distortionCorrection){
        mProgram = createProgram(vs_text_vddc(ExampleCoeficients), fs_text());
        mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
        mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    }else{
        mProgram= createProgram(vs_text(), fs_text());
        mMVPMatrixHandle=(GLuint)glGetUniformLocation((GLuint)mProgram,"uMVPMatrix");
    }
    mColorHandle=(GLuint)glGetUniformLocation((GLuint)mProgram,"uColorVec3");
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");
    mSamplerHandle = glGetUniformLocation (mProgram, "sTexture" );
    glGenTextures(1, mTexture);
    checkGlError("GLRenderTexture");
}

int GLRenderText::convertStringToVECs_UVs(const string text, const float X, const float Y, const float Z,const float charHeight,float *array,const int arrayOffset) {
    float x=X;
    float y=Y;
    float z=Z;
    int counter=0;
    int nVertices=0;
    for(unsigned int j=0; j<text.length(); j++)
    {
        char c=text.at(j);
        int indx=c-32;
        if(indx<0) {
            c='?';
            indx=c-32;
        }
        // Calculate the uv parts
        int row = indx / 16;
        int col = indx % 16;
        //LOGV("%c: indx)%d row)%d col)%d", c,indx,row,col);

        float v = row * CELL_WIDTH_HEIGHT_UV;
        float v2 = v + CELL_WIDTH_HEIGHT_UV;
        float u = col * CELL_WIDTH_HEIGHT_UV;
        float u2 = u + CELL_WIDTH_HEIGHT_UV;

        array[0+counter+arrayOffset] = x;
        array[1+counter+arrayOffset] = y + (charHeight);
        array[2+counter+arrayOffset] = z;
        array[3+counter+arrayOffset] = u+0.001f;
        array[4+counter+arrayOffset] = v+0.001f;

        array[5+counter+arrayOffset] = x;
        array[6+counter+arrayOffset] = y;
        array[7+counter+arrayOffset] = z;
        array[8+counter+arrayOffset] = u+0.001f;
        array[9+counter+arrayOffset] = v2-0.001f;

        array[10+counter+arrayOffset] = x + (charHeight);
        array[11+counter+arrayOffset] = y;
        array[12+counter+arrayOffset] = z;
        array[13+counter+arrayOffset] = u2-0.001f;
        array[14+counter+arrayOffset] = v2-0.001f;


        array[15+counter+arrayOffset] = x;
        array[16+counter+arrayOffset] = y + (charHeight);
        array[17+counter+arrayOffset] = z;
        array[18+counter+arrayOffset] = u+0.001f;
        array[19+counter+arrayOffset] = v+0.001f;

        array[20+counter+arrayOffset] = x+ (charHeight);
        array[21+counter+arrayOffset] = y;
        array[22+counter+arrayOffset] = z;
        array[23+counter+arrayOffset] = u2+0.001f;
        array[24+counter+arrayOffset] = v2-0.001f;

        array[25+counter+arrayOffset] = x + (charHeight);
        array[26+counter+arrayOffset] = y + (charHeight);
        array[27+counter+arrayOffset] = z;
        array[28+counter+arrayOffset] = u2-0.001f;
        array[29+counter+arrayOffset] = v+0.001f;

        counter+=30;
        x+=(float)FONT_WIDTH_PIXELS[indx+32]/(float)CELL_WIDTH_HEIGHT_PIXELS*(float)charHeight;
        nVertices+=6;
    }
    return nVertices;
}

void GLRenderText::convertStringToVECs_UVs(GLTextObj* gltextObj){
    gltextObj->NVertices=convertStringToVECs_UVs(gltextObj->Text, gltextObj->X,gltextObj->Y,gltextObj->Z,
                                                    gltextObj->TextH,gltextObj->VECS_UVS,0);
}

float GLRenderText::getStringLength(const string s, const float charHeight) {
    float l=0;
    for(unsigned int i=0;i<s.length();i++){
        char c=s.at(i);
        int indx=c-32;
        l+=(float)FONT_WIDTH_PIXELS[indx]/(float)CELL_WIDTH_HEIGHT_PIXELS*charHeight;
    }
    return l;
}
void GLRenderText::beforeDraw(const GLuint buffer) const{
#ifdef WIREFRAME
    glLineWidth(4.0f);
#endif
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,mTexture[0]);
    glUniform1i(mSamplerHandle,0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,5*sizeof(float),(GLvoid*)(3*sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void GLRenderText::draw(const glm::mat4x4 ViewM, const  glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices,const float r,const float g,const float b) const {
    if(distortionCorrection){
        glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
        glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    }else{
        glm::mat4x4 VPM= ProjM*ViewM;
        glUniformMatrix4fv(mMVPMatrixHandle, 1, GL_FALSE, glm::value_ptr(VPM));
    }
    float rgb[]={r,g,b};
    glUniform3fv(mColorHandle,1,rgb);
#ifdef WIREFRAME
    glDrawArrays(GL_LINES, verticesOffset, numberVertices);
    glDrawArrays(GL_POINTS, verticesOffset, numberVertices);
#endif
#ifndef WIREFRAME
    glDrawArrays(GL_TRIANGLES, verticesOffset, numberVertices);
#endif
}
void GLRenderText::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_2D,0);
}

void AssetReadFile(JNIEnv* env,jobject assetManagerJAVA,
                   std::string& assetName, std::vector<uint8_t>& buf) {
    AAssetManager* assetManager=AAssetManager_fromJava(env,assetManagerJAVA);
    if(!assetManager){
        LOGV("Cannot create native asset manager");
    }
    AAsset* assetDescriptor = AAssetManager_open(assetManager,
                                                 assetName.c_str(),
                                                 AASSET_MODE_BUFFER);
    size_t fileLength = (size_t)AAsset_getLength(assetDescriptor);
    //LOGV("Size1:%d",(int)fileLength);
    buf.resize(fileLength);
    //int64_t readSize = AAsset_read(assetDescriptor, buf.data(), buf.size());
    AAsset_read(assetDescriptor, buf.data(), buf.size());
    //LOGV("Size2:%d",(int)buf.size());
    AAsset_close(assetDescriptor);
}

void  GLRenderText::loadTextureImage(JNIEnv *env, jobject obj,jobject assetManagerJAVA) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,mTexture[0]);

    //load the assets file into a vector buffer
    std::string assetName="text_atlas.png";
    std::vector<uint8_t> buff;
    AssetReadFile(env,assetManagerJAVA,assetName,buff);

    //convert vector into jbyteArray
    jbyteArray buffJArray;
    buffJArray=env->NewByteArray((jsize)(buff.size()* sizeof(uint8_t)));
    void *temp = env->GetPrimitiveArrayCritical(buffJArray, 0);
    memcpy(temp, buff.data(),buff.size()* sizeof(uint8_t) );
    env->ReleasePrimitiveArrayCritical(buffJArray, temp, 0);
    //call the static JNIHelper->loadPNGIntoTexture() Java function
    std::string className="constantin/fpv_vr/JNIHelper";
    jclass JNIHelper_ = env->FindClass(className.c_str());
    jmethodID loadPNGIntoTexture_=env->GetStaticMethodID(JNIHelper_, "loadPNGIntoTexture", "([B)V");
    env->CallStaticVoidMethod(JNIHelper_, loadPNGIntoTexture_,buffJArray);
    env->DeleteLocalRef(buffJArray);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGlError("loadTexture");
}

