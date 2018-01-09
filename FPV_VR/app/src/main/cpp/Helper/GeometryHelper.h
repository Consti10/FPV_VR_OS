
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>


/**************************************************
 * Functions to create geometry that can be used by OpenGL
 **************************************************/

static const void makeColoredRect(float array[],int arrayOffset,glm::vec3 point,glm::vec3 width,glm::vec3 height,
                                  float r,float g,float b,float a){
    array[arrayOffset   ]=point[0];
    array[arrayOffset+ 1]=point[1];
    array[arrayOffset+ 2]=point[2];
    array[arrayOffset+ 3]=r;
    array[arrayOffset+ 4]=g;
    array[arrayOffset+ 5]=b;
    array[arrayOffset+ 6]=a;
    array[arrayOffset+ 7]=point[0]+height[0];
    array[arrayOffset+ 8]=point[1]+height[1];
    array[arrayOffset+ 9]=point[2]+height[2];
    array[arrayOffset+10]=r;
    array[arrayOffset+11]=g;
    array[arrayOffset+12]=b;
    array[arrayOffset+13]=a;
    array[arrayOffset+14]=point[0]+height[0]+width[0];
    array[arrayOffset+15]=point[1]+height[1]+width[1];
    array[arrayOffset+16]=point[2]+height[2]+width[2];
    array[arrayOffset+17]=r;
    array[arrayOffset+18]=g;
    array[arrayOffset+19]=b;
    array[arrayOffset+20]=a;
    //3
    array[arrayOffset+21]=point[0];
    array[arrayOffset+22]=point[1];
    array[arrayOffset+23]=point[2];
    array[arrayOffset+24]=r;
    array[arrayOffset+25]=g;
    array[arrayOffset+26]=b;
    array[arrayOffset+27]=a;
    array[arrayOffset+28]=point[0]+height[0]+width[0];
    array[arrayOffset+29]=point[1]+height[1]+width[1];
    array[arrayOffset+30]=point[2]+height[2]+width[2];
    array[arrayOffset+31]=r;
    array[arrayOffset+32]=g;
    array[arrayOffset+33]=b;
    array[arrayOffset+34]=a;
    array[arrayOffset+35]=point[0]+width[0];
    array[arrayOffset+36]=point[1]+width[1];
    array[arrayOffset+37]=point[2]+width[2];
    array[arrayOffset+38]=r;
    array[arrayOffset+39]=g;
    array[arrayOffset+40]=b;
    array[arrayOffset+41]=a;
}
static const void makeColoredLine(float array[],int arrayOffset,glm::vec3 point1,glm::vec3 point2,
                                  float r,float g,float b,float a){
    array[arrayOffset   ]=point1[0];
    array[arrayOffset+ 1]=point1[1];
    array[arrayOffset+ 2]=point1[2];
    array[arrayOffset+ 3]=r;
    array[arrayOffset+ 4]=g;
    array[arrayOffset+ 5]=b;
    array[arrayOffset+ 6]=a;
    array[arrayOffset+ 7]=point2[0];
    array[arrayOffset+ 8]=point2[1];
    array[arrayOffset+ 9]=point2[2];
    array[arrayOffset+10]=r;
    array[arrayOffset+11]=g;
    array[arrayOffset+12]=b;
    array[arrayOffset+13]=a;

}

static const void makeTexturedRect(float array[],int arrayOffset,glm::vec3 point,glm::vec3 width,glm::vec3 height,
                                   float u,float v,float uRange,float vRange){
    //|--------------------------------|h
    //|                                |e
    //|                                |i (V)
    //|                                |g
    //|                                |h
    //|--------------------------------|t
    // point           width (U)
    //u is normal, but v is flipped
    array[arrayOffset   ]=point[0];
    array[arrayOffset+ 1]=point[1];
    array[arrayOffset+ 2]=point[2];
    array[arrayOffset+ 3]=u;
    array[arrayOffset+ 4]=v+vRange;
    array[arrayOffset+ 5]=point[0]+height[0];
    array[arrayOffset+ 6]=point[1]+height[1];
    array[arrayOffset+ 7]=point[2]+height[2];
    array[arrayOffset+ 8]=u;
    array[arrayOffset+ 9]=v;
    array[arrayOffset+10]=point[0]+height[0]+width[0];
    array[arrayOffset+11]=point[1]+height[1]+width[1];
    array[arrayOffset+12]=point[2]+height[2]+width[2];
    array[arrayOffset+13]=u+uRange;
    array[arrayOffset+14]=v;
    //3
    array[arrayOffset+15]=point[0];
    array[arrayOffset+16]=point[1];
    array[arrayOffset+17]=point[2];
    array[arrayOffset+18]=u;
    array[arrayOffset+19]=v+vRange;
    array[arrayOffset+20]=point[0]+height[0]+width[0];
    array[arrayOffset+21]=point[1]+height[1]+width[1];
    array[arrayOffset+22]=point[2]+height[2]+width[2];
    array[arrayOffset+23]=u+uRange;
    array[arrayOffset+24]=v;
    array[arrayOffset+25]=point[0]+width[0];
    array[arrayOffset+26]=point[1]+width[1];
    array[arrayOffset+27]=point[2]+width[2];
    array[arrayOffset+28]=u+uRange;
    array[arrayOffset+29]=v+vRange;
}
/*static const void makeTexturedRect(float array[],int arrayOffset,glm::vec3 point,glm::vec3 width,glm::vec3 height,
                                   float u,float v,float uRange,float vRange){
    //|--------------------------------|h
    //|                                |e
    //|                                |i (V)
    //|                                |g
    //|                                |h
    //|--------------------------------|t
    // point           width (U)
    array[arrayOffset   ]=point[0];
    array[arrayOffset+ 1]=point[1];
    array[arrayOffset+ 2]=point[2];
    array[arrayOffset+ 3]=u;
    array[arrayOffset+ 4]=v;
    array[arrayOffset+ 5]=point[0]+height[0];
    array[arrayOffset+ 6]=point[1]+height[1];
    array[arrayOffset+ 7]=point[2]+height[2];
    array[arrayOffset+ 8]=u;
    array[arrayOffset+ 9]=v+vRange;
    array[arrayOffset+10]=point[0]+height[0]+width[0];
    array[arrayOffset+11]=point[1]+height[1]+width[1];
    array[arrayOffset+12]=point[2]+height[2]+width[2];
    array[arrayOffset+13]=u+uRange;
    array[arrayOffset+14]=v+vRange;
    //3
    array[arrayOffset+15]=point[0];
    array[arrayOffset+16]=point[1];
    array[arrayOffset+17]=point[2];
    array[arrayOffset+18]=u;
    array[arrayOffset+19]=v;
    array[arrayOffset+20]=point[0]+height[0]+width[0];
    array[arrayOffset+21]=point[1]+height[1]+width[1];
    array[arrayOffset+22]=point[2]+height[2]+width[2];
    array[arrayOffset+23]=u+uRange;
    array[arrayOffset+24]=v+vRange;
    array[arrayOffset+25]=point[0]+width[0];
    array[arrayOffset+26]=point[1]+width[1];
    array[arrayOffset+27]=point[2]+width[2];
    array[arrayOffset+28]=u+uRange;
    array[arrayOffset+29]=v;
}*/

static const void makeVideoCanvas(float array[],int arrayOffset,glm::vec3 point,float width,float height){
    makeTexturedRect(array,arrayOffset,point,glm::vec3(width,0.0f,0.0f),glm::vec3(0.0f,height,0.0f),0,0,1,1);
}

static const void makeTesselatedVideoCanvas(float array[],int arrayOffset,glm::vec3 point,float width,float height,int tesselation){
    float uRange=1.0f;
    float vRange=1.0f;
    int tesselationX=tesselation;
    int tesselationY=tesselation;
    float subW=width/(float)tesselationX;
    float subH=height/(float)tesselationY;
    float subURange=uRange/(float)tesselationX;
    float subVRange=vRange/(float)tesselationY;
    for(int i=0;i<tesselationX;i++){
        float xPos=point[0]+(i*subW);
        for(int i2=0;i2<tesselationY;i2++){
            float yPos=point[1]+(i2*subH);
            makeTexturedRect(array,arrayOffset,glm::vec3(xPos,yPos,point[2]),glm::vec3(subW,0.0f,0.0f),glm::vec3(0.0f,subH,0.0f),
                             i*subURange,1.0f-i2*subVRange,subURange,subVRange);
            arrayOffset+=30;
        }
    }
}
static const void makeColoredTriangle1(float array[],int arrayOffset,glm::vec3 point,float width, float height,
                                      float r,float g,float b,float a) {
    array[arrayOffset + 0] = point[0];
    array[arrayOffset + 1] = point[1];
    array[arrayOffset + 2] = point[2];
    array[arrayOffset + 3] = r;
    array[arrayOffset + 4] = g;
    array[arrayOffset + 5] = b;
    array[arrayOffset + 6] = a;
    array[arrayOffset + 7] = point[0]+width;
    array[arrayOffset + 8] = point[1];
    array[arrayOffset + 9] = point[2];
    array[arrayOffset + 10] = r;
    array[arrayOffset + 11] = g;
    array[arrayOffset + 12] = b;
    array[arrayOffset + 13] = a;
    array[arrayOffset + 14] = point[0]+width/2.0f;
    array[arrayOffset + 15] = point[1]+height;
    array[arrayOffset + 16] = point[2];
    array[arrayOffset + 17] = r;
    array[arrayOffset + 18] = g;
    array[arrayOffset + 19] = b;
    array[arrayOffset + 20] = a;
}

static const void makeColoredTriangle2(float array[],int arrayOffset,glm::vec3 point1,glm::vec3 point2, glm::vec3 point3,
                                       float r,float g,float b,float a) {
    array[arrayOffset + 0] = point1[0];
    array[arrayOffset + 1] = point1[1];
    array[arrayOffset + 2] = point1[2];
    array[arrayOffset + 3] = r;
    array[arrayOffset + 4] = g;
    array[arrayOffset + 5] = b;
    array[arrayOffset + 6] = a;
    array[arrayOffset + 7] = point2[0];
    array[arrayOffset + 8] = point2[1];
    array[arrayOffset + 9] = point2[2];
    array[arrayOffset + 10] = r;
    array[arrayOffset + 11] = g;
    array[arrayOffset + 12] = b;
    array[arrayOffset + 13] = a;
    array[arrayOffset + 14] = point3[0];
    array[arrayOffset + 15] = point3[1];
    array[arrayOffset + 16] = point3[2];
    array[arrayOffset + 17] = r;
    array[arrayOffset + 18] = g;
    array[arrayOffset + 19] = b;
    array[arrayOffset + 20] = a;
}

static const void makeColoredTriangle3(float array[],int arrayOffset,glm::vec3 basePoint,glm::vec3 vec1, glm::vec3 vec2,
                                       float r,float g,float b,float a){
    makeColoredTriangle2(array,arrayOffset,basePoint,basePoint+vec1,basePoint+vec2,r,g,b,a);
}