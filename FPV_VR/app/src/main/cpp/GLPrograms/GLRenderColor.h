/*****************************************************
 * Abstraction for drawing geometry in form of x,y,z, r,g,b,a vertices
 * The application only has to create and fill a VBO with matching data.
 * Setting up the rendering variables and rendering is done via beforeDraw(),draw() and afterDraw().
 ****************************************************/
#ifndef GLRENDERCOLOR
#define GLRENDERCOLOR

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLRenderColor {
private:
    GLuint mProgram;
    GLuint mMVPMatrixHandle,mPositionHandle,mColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    bool distortionCorrection;
public:
    GLRenderColor(const bool enableDist);
    void beforeDraw(const GLuint buffer) const;
    void draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices) const;
    void afterDraw() const;
};

#endif
