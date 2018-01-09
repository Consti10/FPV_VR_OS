
#ifndef OSDRENDERER
#define OSDRENDERER

#include <GLES2/gl2.h>
#include <GLRenderText.h>
#include <GLRenderGeometry.h>
#include <TelemetryReceiver.h>
#include <GLRenderLine.h>
#include "ModelRollPitch.h"
#include "TextElements.h"
#include "CompasLadder.h"
#include "HeightLadder.h"

#define DEBUG_POSITION
using namespace std;

class OSDRenderer {
#ifdef DEBUG_POSITION
    GLuint mGLDebugB[1];
#endif
public:
    OSDRenderer(TelemetryReceiver* telemetryReceiver,const GLRenderColor* glRenderColor,const GLRenderLine* glRenderLine,const GLRenderText* glRenderText);
    void placeGLElementsMono(float videoX,float videoY,float videoZ,float videoW,float videoH,int strokeW);
    void placeGLElementsStereo(float videoX,float videoY,float videoZ,float videoW,float videoH,int strokeW);
    //
    void updateAndDrawElements(glm::mat4x4 ViewM,glm::mat4x4 ViewM2,glm::mat4x4 ProjM,bool create3PPerspective);
    void drawElementsGL(glm::mat4x4 ViewM,glm::mat4x4 ViewM2, glm::mat4x4 ProjM,bool create3PPerspective);
    void updateElementsGL();

    void stop();
    void start();
private:
    TelemetryReceiver* mTelemtryReceiver;
    const GLRenderText* mGLRenderText;
    const GLRenderColor* mGLRenderColor;
    const GLRenderLine* mGLRenderLine;
    ModelRollPitch* mModelRollPitch=NULL;
    TextElements* mTextElements=NULL;
    CompasLadder* mCompasLadder=NULL;
    HeightLadder* mHeightLadder=NULL;
    int heightLadderTelemetryHeightSourceVal=0;
};

#endif

