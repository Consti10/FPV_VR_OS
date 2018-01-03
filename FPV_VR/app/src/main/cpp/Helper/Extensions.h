//
// Created by Constantin on 30.10.2017.
//

#ifndef OSDTESTER_EXTENSIONS_H
#define OSDTESTER_EXTENSIONS_H

#include <sched.h>
#include <unistd.h>
#include <android/log.h>
#include <sys/syscall.h>
#include <sys/errno.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>

#define PrivTAG "Extensions"
#define PrivLOG(...) __android_log_print(ANDROID_LOG_VERBOSE, PrivTAG, __VA_ARGS__)

PFNGLSTARTTILINGQCOMPROC	glStartTilingQCOM_;
PFNGLENDTILINGQCOMPROC		glEndTilingQCOM_;

typedef void (GL_APIENTRYP PFNGLINVALIDATEFRAMEBUFFER_) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
PFNGLINVALIDATEFRAMEBUFFER_	glInvalidateFramebuffer_;

PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR_;
PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR_;
PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR_;
PFNEGLSIGNALSYNCKHRPROC eglSignalSyncKHR_;
PFNEGLGETSYNCATTRIBKHRPROC eglGetSyncAttribKHR_;


static const void initQCOMTiling(){
    glStartTilingQCOM_ = (PFNGLSTARTTILINGQCOMPROC)eglGetProcAddress("glStartTilingQCOM");
    glEndTilingQCOM_ = (PFNGLENDTILINGQCOMPROC)eglGetProcAddress("glEndTilingQCOM");
}

static const void glStartTilingQCOM(int x,int y,int width,int height) {
    glStartTilingQCOM_( (GLuint)x,(GLuint)y,(GLuint)width,(GLuint)height, 0 );
}

static const void glEndTilingQCOM() {
    glEndTilingQCOM_( GL_COLOR_BUFFER_BIT0_QCOM );
}


static const void initOtherExtensions(){
    glInvalidateFramebuffer_  = (PFNGLINVALIDATEFRAMEBUFFER_)eglGetProcAddress("glInvalidateFramebuffer");
    eglCreateSyncKHR_ = (PFNEGLCREATESYNCKHRPROC)eglGetProcAddress( "eglCreateSyncKHR" );
    eglDestroySyncKHR_ = (PFNEGLDESTROYSYNCKHRPROC)eglGetProcAddress( "eglDestroySyncKHR" );
    eglClientWaitSyncKHR_ = (PFNEGLCLIENTWAITSYNCKHRPROC)eglGetProcAddress( "eglClientWaitSyncKHR" );
    eglSignalSyncKHR_ = (PFNEGLSIGNALSYNCKHRPROC)eglGetProcAddress( "eglSignalSyncKHR" );
    eglGetSyncAttribKHR_ = (PFNEGLGETSYNCATTRIBKHRPROC)eglGetProcAddress( "eglGetSyncAttribKHR" );
}

/*static const void glInvalidateFramebuffer(const invalidateTarget_t isFBO,const bool colorBuffer, const bool depthBuffer){
    const int offset = (int)!colorBuffer;
    const int count = (int)colorBuffer + ((int)depthBuffer)*2;

    const GLenum fboAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
    const GLenum attachments[3] = { GL_COLOR_EXT, GL_DEPTH_EXT, GL_STENCIL_EXT };
    glInvalidateFramebuffer_( GL_FRAMEBUFFER, count, ( isFBO == INV_FBO ? fboAttachments : attachments ) + offset );
}*/
static const void glInvalidateFramebuffer(){
    int count=3;
    const GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
    glInvalidateFramebuffer_( GL_FRAMEBUFFER, count, attachments);
}

static const void setAffinity(int core){
    cpu_set_t  cpuset;
    CPU_ZERO(&cpuset);       //clears the cpuset
    CPU_SET( core, &cpuset); //set CPU x on cpuset*/

    long err,syscallres;
    pid_t pid=gettid();
    syscallres=syscall(__NR_sched_setaffinity,pid, sizeof(cpuset),&cpuset);
    if(syscallres) {
        err = errno;
        PrivLOG("Error sched_setaffinity");
    }
}

#endif //OSDTESTER_EXTENSIONS_H
