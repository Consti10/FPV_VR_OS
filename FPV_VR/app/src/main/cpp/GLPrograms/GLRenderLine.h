//
// Created by Constantin on 06.01.2018.
//

#ifndef FPV_VR_GLRENDERLINE_H
#define FPV_VR_GLRENDERLINE_H

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLRenderLine {
private:
    GLuint mProgram;
    GLuint mMVPMatrixHandle,mPositionHandle,mColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    bool distortionCorrection;
public:
    GLRenderLine(const bool enableDist);
    void beforeDraw(const GLuint buffer) const;
    void draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices,const int strokeW) const;
    void afterDraw() const;
};


#endif //FPV_VR_GLRENDERLINE_H
