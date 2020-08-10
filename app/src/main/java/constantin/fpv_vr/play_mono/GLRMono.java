package constantin.fpv_vr.play_mono;

import android.content.Context;
import android.opengl.EGL14;
import android.opengl.GLSurfaceView;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.fpv_vr.R;
import constantin.renderingx.core.old.MyEGLConfigChooser;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.video_player.VideoSettings;

import static constantin.renderingx.core.old.MyEGLConfigChooser.EGL_ANDROID_front_buffer_auto_refresh;


/*
 * Renders OSD and/or video in monoscopic view
 */
@SuppressWarnings("WeakerAccess")
public class GLRMono implements GLSurfaceView.Renderer{
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
    //Optional, only when playing video that cannot be displayed by a 'normal' android surface
    //(e.g. 360° video)
    private final SurfaceTextureHolder mVideoSurfaceHolder;
    private final boolean disableVSYNC;

    public GLRMono(final AppCompatActivity context,final TelemetryReceiver telemetryReceiver, GvrApi gvrApi, final int videoMode, final boolean renderOSD,
                   final boolean disableVSYNC){
        mContext=context;
        this.disableVSYNC=disableVSYNC;
        if(videoMode!= VideoSettings.VIDEO_MODE_2D_MONOSCOPIC){
            mVideoSurfaceHolder=new SurfaceTextureHolder(context,null);
        }else{
            mVideoSurfaceHolder=null;
        }
        nativeGLRendererMono=nativeConstruct(context,telemetryReceiver.getNativeInstance(),gvrApi.getNativeGvrContext(),videoMode,renderOSD);
    }

    public final SurfaceTextureHolder getVideoSurfaceHolder(){
        return mVideoSurfaceHolder;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        if(mVideoSurfaceHolder!=null){
            mVideoSurfaceHolder.createOnOpenGLThread();
        }
        if(disableVSYNC){
            MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
            MyEGLConfigChooser.setEglSurfaceAttrib(EGL_ANDROID_front_buffer_auto_refresh,EGL14.EGL_TRUE);
            EGL14.eglSwapBuffers(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
        }
        nativeOnSurfaceCreated(nativeGLRendererMono,mContext,mVideoSurfaceHolder==null ? 0 : mVideoSurfaceHolder.getTextureId());
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        final float video360FOV=mContext.getSharedPreferences("pref_video",Context.MODE_PRIVATE).getFloat(mContext.getString(R.string.VS_360_VIDEO_FOV),50);
        nativeOnSurfaceChanged(nativeGLRendererMono,width,height,video360FOV);
        //MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if(mVideoSurfaceHolder!=null){
           mVideoSurfaceHolder.getSurfaceTexture().updateTexImage();
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

    public void setVideoRatio(int videoW, int videoH) {
        nativeOnVideoRatioChanged(nativeGLRendererMono,videoW,videoH);
    }

    public void setHomeOrientation(){
        nativeSetHomeOrientation360(nativeGLRendererMono);
    }
}
