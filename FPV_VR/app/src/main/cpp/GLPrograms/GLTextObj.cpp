
#include <pthread.h>
#include "GLTextObj.h"

GLTextObj::GLTextObj(std::string text, float x, float y, float z, float textH, float r, float g,
                     float b) {
    Text=text;
    X=x;
    Y=y;
    Z=z;
    TextH=textH;
    R=r;
    G=g;
    B=b;
    newVecs_UVScalculated=false;
    pthread_mutex_init(&lock,NULL);
}
