/********************************************
 * Holds all OpenGL Shaders and helper functions
 * vs=Vertex Shader fs=Fragment Shader vddc=Vertex Displacement Distortion Correction
 * *******************************************/
#include <string>
#include <GLES2/gl2.h>
#include "android/log.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <sstream>

#define TAGGLRenderTexture "GLRenderTexture"
#define LOGVGLRenderTexture(...) __android_log_print(ANDROID_LOG_VERBOSE, TAGGLRenderTexture, __VA_ARGS__)

using namespace std;

//#define WIREFRAME

template <typename T> string tostr(const T& t) {
    ostringstream os;
    os<<t;
    return os.str();
}

static float ExampleCoeficients[]={0.7593979f,-0.34376323f,-0.14616828f,0.75306076f,-0.98268133f,0.6100197f,-0.1503692f};

static const GLuint loadShader(GLenum type, string shaderCode){
    GLuint shader = glCreateShader(type);
    // add the source code to the shader and compile it
    const char *c_str = shaderCode.c_str();
    glShaderSource(shader, 1, &c_str, NULL);
    glCompileShader(shader);
    return shader;
}
static void checkGlError(string op) {
    int error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        LOGVGLRenderTexture("%s GLError:%d",op.c_str(),error);
    }
}
static GLuint createProgram(string vertexSource, string fragmentSource) {
    auto vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) {
        return 0;
    }
    auto pixelShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (pixelShader == 0) {
        return 0;
    }
    auto program = glCreateProgram();
    if (program != 0) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        int linkStatus[1];
        glGetProgramiv(program,GL_LINK_STATUS,linkStatus);
        if (linkStatus[0] != GL_TRUE) {
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}
//GEOMETRY####GEOMETRY####GEOMETRY####GEOMETRY####GEOMETRY####GEOMETRY####GEOMETRY####GEOMETRY####GEOMETRY####GEOMETRY####
static const string vs_geometry(){
    string s="";
    s.append("uniform mat4 uMVPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec4 aColor;\n");
    s.append("varying vec4 vColor;\n");
    s.append("void main(){\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("vColor = aColor;\n");
    s.append("gl_Position = uMVPMatrix* aPosition;\n");
    s.append("}\n");
    return s;
}
static const string vs_geometry_vddc(float coeficients[]){
    string s="";
    s.append("uniform mat4 uMVMatrix;\n");
    s.append("uniform mat4 uPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec4 aColor;\n");
    s.append("varying vec4 vColor;\n");
    s.append("float r2;\n");
    s.append("vec4 pos;\n");
    s.append("float ret;\n");
    s.append("float _MaxRadSq=");
    s.append(tostr(coeficients[0]));
    s.append(";\n");
    //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
    s.append("vec4 _Undistortion=vec4(");
    s.append(tostr(coeficients[1]));
    s.append(",");
    s.append(tostr(coeficients[2]));
    s.append(",");
    s.append(tostr(coeficients[3]));
    s.append(",");
    s.append(tostr(coeficients[4]));
    s.append(");\n");
    s.append("vec2 _Undistortion2=vec2(");
    s.append(tostr(coeficients[5]));
    s.append(",");
    s.append(tostr(coeficients[6]));
    s.append(");\n");
    s.append("void main(){\n");
    s.append("  pos=uMVMatrix * aPosition;\n");
    s.append("  r2=clamp(dot(pos.xy,pos.xy)/(pos.z*pos.z),0.0,_MaxRadSq);\n");
    s.append("  ret = 0.0;\n");
    s.append("  ret = r2 * (ret + _Undistortion2.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion2.x);\n");
    s.append("  ret = r2 * (ret + _Undistortion.w);\n");
    s.append("  ret = r2 * (ret + _Undistortion.z);\n");
    s.append("  ret = r2 * (ret + _Undistortion.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion.x);\n");
    s.append("  pos.xy*=1.0+ret;\n");
    s.append("  gl_Position=uPMatrix*pos;\n");
    s.append("  vColor = aColor;\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("}\n");
    //LOGV2("%s",s.c_str());
    return s;
}
static const string fs_geometry(){
    string s="";
    s.append("precision mediump float;\n");
    s.append("varying vec4 vColor;\n");
    s.append("void main(){\n");
    s.append("gl_FragColor = vColor;\n");
#ifdef WIREFRAME
    s.append("gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n");
#endif
    s.append("}\n");
    s.append("\n");
    return s;
}
//TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###TEXT###
static const string vs_text(){
    string s="";
    s.append("uniform mat4 uMVPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec2 aTexCoord;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("\n");
    s.append("void main() {\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("gl_Position = uMVPMatrix * aPosition;\n");
    s.append("vTexCoord = aTexCoord;\n");
    s.append("}");
    return s;
}
static const string vs_text_vddc(float coeficients[]){
    string s="";
    s.append("uniform mat4 uMVMatrix;\n");
    s.append("uniform mat4 uPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec2 aTexCoord;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("float r2;\n");
    s.append("vec4 pos;\n");
    s.append("float ret;\n");
    s.append("float _MaxRadSq=");
    s.append(tostr(coeficients[0]));
    s.append(";\n");
    //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
    s.append("vec4 _Undistortion=vec4(");
    s.append(tostr(coeficients[1]));
    s.append(",");
    s.append(tostr(coeficients[2]));
    s.append(",");
    s.append(tostr(coeficients[3]));
    s.append(",");
    s.append(tostr(coeficients[4]));
    s.append(");\n");
    s.append("vec2 _Undistortion2=vec2(");
    s.append(tostr(coeficients[5]));
    s.append(",");
    s.append(tostr(coeficients[6]));
    s.append(");\n");
    s.append("void main() {\n");
    s.append("  pos=uMVMatrix * aPosition;\n");
    s.append("  r2=clamp(dot(pos.xy,pos.xy)/(pos.z*pos.z),0.0,_MaxRadSq);\n");
    s.append("  ret = 0.0;\n");
    s.append("  ret = r2 * (ret + _Undistortion2.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion2.x);\n");
    s.append("  ret = r2 * (ret + _Undistortion.w);\n");
    s.append("  ret = r2 * (ret + _Undistortion.z);\n");
    s.append("  ret = r2 * (ret + _Undistortion.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion.x);\n");
    s.append("  pos.xy*=1.0+ret;\n");
    s.append("  gl_Position=uPMatrix*pos;\n");
    s.append("  vTexCoord = aTexCoord;\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("}");
    return s;
}
static const string fs_text(){
    string s="";
    s.append("precision mediump float;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("uniform sampler2D sTexture;\n");
    s.append("uniform vec3 uColorVec3;\n");
    s.append("void main() {\n");
    /*s.append("gl_FragColor = texture2D( sTexture, vTexCoord ) * vec4(uColorVec3,1.0);\n");
    s.append("gl_FragColor.rgb *= vec4(uColorVec3,1.0).a;\n");
    //s.append("if(gl_FragColor.rgb!=vec3(0.0,0.0,0.0)){\n");
    s.append("if(gl_FragColor.rgb==vec3(1.0,1.0,1.0)){\n");
    s.append("discard;\n");
    s.append("}\n");*/
    //s.append("vec4 textVal = texture2D( sTexture, vTexCoord );\n");
    //s.append("gl_FragColor = textVal;\n");
    //s.append("vec4 tex = texture2D ( sTexture, vTexCoord );\n");
    //s.append("gl_FragColor = tex+vec4(0.5,0,0,1)*tex.a;\n");
    //s.append("gl_FragColor = vec4(tex.r, tex.g, tex.b, tex.a) + vec4(0.5, 0, 0, 1)*tex.a;\n");
    s.append("gl_FragColor = texture2D( sTexture, vTexCoord );\n");
    //s.append("gl_FragColor.rgb =uColorVec3;\n");
#ifdef WIREFRAME
    s.append("gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n");
#endif
    s.append("}\n");
    return s;
}
//TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###TEXTURE###
static const string vs_texture(){
    string s;
    s.append("uniform mat4 uMVPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec2 aTexCoord;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("void main() {\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("gl_Position = uMVPMatrix * aPosition;\n");
    s.append("vTexCoord = aTexCoord;\n");
    s.append("}\n");
    return s;
}
static const string vs_texture_vddc(float coeficients[]){
    string s="";
    s.append("uniform mat4 uMVMatrix;\n");
    s.append("uniform mat4 uPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec2 aTexCoord;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("float r2;\n");
    s.append("vec4 pos;\n");
    s.append("float ret;\n");
    s.append("float _MaxRadSq=");
    s.append(tostr(coeficients[0]));
    s.append(";\n");
    //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
    s.append("vec4 _Undistortion=vec4(");
    s.append(tostr(coeficients[1]));
    s.append(",");
    s.append(tostr(coeficients[2]));
    s.append(",");
    s.append(tostr(coeficients[3]));
    s.append(",");
    s.append(tostr(coeficients[4]));
    s.append(");\n");
    s.append("vec2 _Undistortion2=vec2(");
    s.append(tostr(coeficients[5]));
    s.append(",");
    s.append(tostr(coeficients[6]));
    s.append(");\n");
    s.append("void main() {\n");
    s.append("  pos=uMVMatrix * aPosition;\n");
    s.append("  r2=clamp(dot(pos.xy,pos.xy)/(pos.z*pos.z),0.0,_MaxRadSq);\n");
    s.append("  ret = 0.0;\n");
    s.append("  ret = r2 * (ret + _Undistortion2.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion2.x);\n");
    s.append("  ret = r2 * (ret + _Undistortion.w);\n");
    s.append("  ret = r2 * (ret + _Undistortion.z);\n");
    s.append("  ret = r2 * (ret + _Undistortion.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion.x);\n");
    s.append("  pos.xy*=1.0+ret;\n");
    s.append("  gl_Position=uPMatrix*pos;\n");
    s.append("  vTexCoord = aTexCoord;\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("}\n");
    return s;
}
static const string fs_texture(){
    string s;
    s.append("precision mediump float;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("uniform sampler2D sTexture;\n");
    s.append("void main() {\n");
    s.append("gl_FragColor = texture2D( sTexture, vTexCoord );\n");
#ifdef WIREFRAME
    s.append("gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n");
#endif
    s.append("}\n");
    return s;
}
//TEXTUREEXTERNAL###TEXTUREEXTERNAL###TEXTUREEXTERNAL###TEXTUREEXTERNAL###TEXTUREEXTERNAL###TEXTUREEXTERNAL###TEXTUREEXTERNAL###
static const string vs_textureExt(){
    string s;
    s.append("uniform mat4 uMVPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec2 aTexCoord;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("void main() {\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("gl_Position = uMVPMatrix * aPosition;\n");
    s.append("vTexCoord = aTexCoord;\n");
    s.append("}\n");
    return s;
}
static const string vs_textureExt_vddc(float coeficients[]){
    string s;
    s.append("uniform mat4 uMVMatrix;\n");
    s.append("uniform mat4 uPMatrix;\n");
    s.append("attribute vec4 aPosition;\n");
    s.append("attribute vec2 aTexCoord;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("float r2;\n");
    s.append("vec4 pos;\n");
    s.append("float ret;\n");
    s.append("float _MaxRadSq=");
    s.append(tostr(coeficients[0]));
    s.append(";\n");
    //There is no vec6 data type. Therefore, we use 1 vec4 and 1 vec2. Vec4 holds k1,k2,k3,k4 and vec6 holds k5,k6
    s.append("vec4 _Undistortion=vec4(");
    s.append(tostr(coeficients[1]));
    s.append(",");
    s.append(tostr(coeficients[2]));
    s.append(",");
    s.append(tostr(coeficients[3]));
    s.append(",");
    s.append(tostr(coeficients[4]));
    s.append(");\n");
    s.append("vec2 _Undistortion2=vec2(");
    s.append(tostr(coeficients[5]));
    s.append(",");
    s.append(tostr(coeficients[6]));
    s.append(");\n");
    s.append("void main() {\n");
    s.append("  pos=uMVMatrix * aPosition;\n");
    s.append("  r2=clamp(dot(pos.xy,pos.xy)/(pos.z*pos.z),0.0,_MaxRadSq);\n");
    s.append("  ret = 0.0;\n");
    s.append("  ret = r2 * (ret + _Undistortion2.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion2.x);\n");
    s.append("  ret = r2 * (ret + _Undistortion.w);\n");
    s.append("  ret = r2 * (ret + _Undistortion.z);\n");
    s.append("  ret = r2 * (ret + _Undistortion.y);\n");
    s.append("  ret = r2 * (ret + _Undistortion.x);\n");
    s.append("  pos.xy*=1.0+ret;\n");
    s.append("  gl_Position=uPMatrix*pos;\n");
    s.append("  vTexCoord = aTexCoord;\n");
#ifdef WIREFRAME
    s.append("gl_PointSize=15.0;");
#endif
    s.append("}\n");
    return s;
}
static const string fs_textureExt(){
    string s;
    s.append("#extension GL_OES_EGL_image_external : require\n");
    s.append("precision mediump float;\n");
    s.append("varying vec2 vTexCoord;\n");
    s.append("uniform samplerExternalOES sTextureExt;\n");
    s.append("void main() {\n");
    s.append("gl_FragColor = texture2D( sTextureExt, vTexCoord );\n");
    //s.append("gl_FragColor.rgb = vec3(1.0,1.0,1.0);\n");
#ifdef WIREFRAME
    s.append("gl_FragColor.rgb=vec3(1.0,1.0,1.0);\n");
#endif
    s.append("}\n");
    return s;
}