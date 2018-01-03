/********************************************************************
 * Uses multithreading to reduce the processing power needed on the OpenGL thread
 * ******************************************************************/
#ifndef TEXTELEMENTS
#define TEXTELEMENTS

#include <vector>
#include <GLES2/gl2.h>
#include <GLRenderText.h>
#include <GLRenderColor.h>
#include <TelemetryReceiver.h>
#include <thread>
#include <atomic>


//#define DEBUG_POSITION

/*struct EnableElements{
    bool DFPS=true;
    bool GLFPS=true;
    bool TIME=true;
    bool RX1=true;
    bool RX2=true;
    bool RX3=true;
    bool BATT_P=true;
    bool BATT_V=true;
    bool BATT_A=true;
    bool BATT_AH=true;
    bool HOME_D=true;
    bool SPEED=true;
    bool LAT=true;
    bool LON=true;
    bool HEIGHT_B=true;
    bool HEIGHT_GPS=true;
};*/
class TextElements {
private:
#ifdef DEBUG_POSITION
    GLuint mGLDebugB[1];
#endif
    const GLRenderText* mGLRenderText;
    const GLRenderColor* mGLRenderColor;
    //vector<std::unique_ptr<GLTextObj*>> mTextStack;
    vector<GLTextObj*> mTextStack;
    thread* mCircularRefreshT=NULL;
    //The start() function may be called from multiple threads, e.g.
    //from the OpenGL thread, or from the UI thread
    pthread_mutex_t runningLock;
    std::atomic<bool> running;
    bool textElementsVertexDataAlreadyCreated;
    TelemetryReceiver* mTelemetryR;
    GLuint mGLTextB[1];
    void circularRefreshThread();
    string getStringForTE(int TE);
    int getNActiveTE();
    //int nActiveTE=0;
    int64_t flightStartMS;
public:
    TextElements(const GLRenderText* glRenderText,const GLRenderColor* glRenderColor,TelemetryReceiver* telemetryR);
    ~TextElements();
    void setWorldPosition(float x,float y,float z,float width,float height,float textH);
    void startUpdating();
    void stopUpdating();
    void updateElementsGL();
    void drawElementsGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);

    const static int TE_DFPS=0;
    const static int TE_GLFPS=1;
    const static int TE_TIME=2;
    const static int TE_RX1=3;
    const static int TE_RX2=4;
    const static int TE_RX3=5;
    const static int TE_BATT_P=6;
    const static int TE_BATT_V=7;
    const static int TE_BATT_A=8;
    const static int TE_BATT_AH=9;
    const static int TE_HOME_D=10;
    const static int TE_SPEED_V=11;
    const static int TE_SPEED_H=12;
    const static int TE_LAT=13;
    const static int TE_LON=14;
    const static int TE_HEIGHT_B=15;
    const static int TE_HEIGHT_GPS=16;
    const static int TE_N_SAT=17;

    static const int MAX_N_TEXT_E=18;

    bool enable[MAX_N_TEXT_E];

    int linkVecToTE[MAX_N_TEXT_E];

};

#endif
