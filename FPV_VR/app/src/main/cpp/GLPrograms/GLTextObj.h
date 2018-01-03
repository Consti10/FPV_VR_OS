
#ifndef GLTEXTOBJ
#define GLTEXTOBJ

#include <string>
#include <GLES2/gl2.h>

class GLTextObj {
public:
    static const int MAX_LENGTH=15;
    GLTextObj(std::string text,float x,float y,float z,float textH,float r,float g,float b);
    std::string Text;
    float VECS_UVS[MAX_LENGTH*6*5];
    float R,G,B;
    float X,Y,Z;
    float TextH;
    int NVertices;

    GLuint mGLBuffer[1];

    bool newVecs_UVScalculated;

    pthread_mutex_t lock;
};


#endif //GLTEXTOBJ
