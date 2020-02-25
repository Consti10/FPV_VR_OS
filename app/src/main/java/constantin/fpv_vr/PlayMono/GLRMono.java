package constantin.fpv_vr.PlayMono;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.google.vr.ndk.base.GvrApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.fpv_vr.R;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.ISurfaceTextureAvailable;
import constantin.video.core.IVideoParamsChanged;

import static constantin.renderingx.core.MyEGLWindowSurfaceFactory.EGL_ANDROID_front_buffer_auto_refresh;

/*
 * Renders OSD and/or video in monoscopic view
 */
public class GLRMono implements GLSurfaceView.Renderer, IVideoParamsChanged{
    static final int VIDEO_MODE_2D_MONOSCOPIC=0;
    static {
        System.loadLibrary("GLRMono");
    }
    native long nativeConstruct(Context context,long nativeTelemetryReceiver,long nativeGvrContext,int videoMode,boolean enableOSD);
    native void nativeDelete(long glRendererMonoP);
    native void nativeOnSurfaceCreated(long glRendererMonoP,Context androidContext,int optionalVideoTexture);
    native void nativeOnSurfaceChanged(long glRendererMonoP,int width,int height,float optionalVideo360FOV);
    native void nativeOnDrawFrame(long glRendererMonoP);
    native void nativeSetHomeOrientation360(long glRendererMonoP);
    private native void nativeOnVideoRatioChanged(long glRenderer,int videoW,int videoH);


    private final long nativeGLRendererMono;
    private final Context mContext;
    private final TelemetryReceiver telemetryReceiver;
    //Optional, only when playing video that cannot be displayed by a 'normal' android surface
    //(e.g. 360° video)
    private SurfaceTexture mSurfaceTexture;
    private final int videoMode;
    private final boolean disableVSYNC;
    private final ISurfaceTextureAvailable iSurfaceTextureAvailable;

    public GLRMono(final Context context,final ISurfaceTextureAvailable iSurfaceTextureAvailable, final TelemetryReceiver telemetryReceiver, GvrApi gvrApi, final int videoMode, final boolean renderOSD,
                   final boolean disableVSYNC){
        this.iSurfaceTextureAvailable=iSurfaceTextureAvailable;
        this.videoMode=videoMode;
        mContext=context;
        this.disableVSYNC=disableVSYNC;
        this.telemetryReceiver=telemetryReceiver;
        nativeGLRendererMono=nativeConstruct(context,telemetryReceiver.getNativeInstance(),gvrApi!=null ? gvrApi.getNativeGvrContext() : 0,videoMode,renderOSD);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        int mGLTextureVideo=0;
        if(videoMode!=VIDEO_MODE_2D_MONOSCOPIC){
            int[] videoTexture=new int[1];
            GLES20.glGenTextures(1, videoTexture, 0);
            mGLTextureVideo = videoTexture[0];
            mSurfaceTexture = new SurfaceTexture(mGLTextureVideo,false);
            iSurfaceTextureAvailable.onSurfaceTextureAvailable(mSurfaceTexture);
        }
        if(disableVSYNC){
            MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
            MyEGLConfigChooser.setEglSurfaceAttrib(EGL_ANDROID_front_buffer_auto_refresh,EGL14.EGL_TRUE);
            EGL14.eglSwapBuffers(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
        }
        nativeOnSurfaceCreated(nativeGLRendererMono,mContext,mGLTextureVideo);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        final float video360FOV=mContext.getSharedPreferences("pref_video",Context.MODE_PRIVATE).getFloat(mContext.getString(R.string.VS_360_VIDEO_FOV),50);
        nativeOnSurfaceChanged(nativeGLRendererMono,width,height,video360FOV);
        //MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if(mSurfaceTexture!=null){
            mSurfaceTexture.updateTexImage();
        }
        nativeOnDrawFrame(nativeGLRendererMono);
    }


    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeGLRendererMono);
        } finally {
            super.finalize();
        }
    }

    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {
        nativeOnVideoRatioChanged(nativeGLRendererMono,videoW,videoH);
    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        if(telemetryReceiver!=null){
            telemetryReceiver.setDecodingInfo(decodingInfo.currentFPS,decodingInfo.currentKiloBitsPerSecond,decodingInfo.avgParsingTime_ms,decodingInfo.avgWaitForInputBTime_ms,
                    decodingInfo.avgHWDecodingTime_ms);
        }
    }

    public void setHomeOrientation(){
        nativeSetHomeOrientation360(nativeGLRendererMono);
    }
}
