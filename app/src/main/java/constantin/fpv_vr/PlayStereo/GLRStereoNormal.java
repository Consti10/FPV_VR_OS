package constantin.fpv_vr.PlayStereo;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLExt;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.MVideoPlayer;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoNative.VideoNative;

import static constantin.renderingx.core.MyEGLWindowSurfaceFactory.EGL_ANDROID_front_buffer_auto_refresh;

/** Open GL renderer stereo in normal mode (normal==no SuperSync or daydream, but possibly VSYNC disabled)
 * Description: see AStereoNormal
 */

public class GLRStereoNormal implements GLSurfaceView.Renderer, IVideoParamsChanged {
    static {
        System.loadLibrary("GLRStereoNormal");
    }
    private native long nativeConstruct(Context context,long telemetryReceiver,long nativeGvrContext,int videoMode);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnSurfaceCreated(long glRendererStereoP,int videoTexture,Context androidContext);
    private native void nativeOnSurfaceChanged(long glRendererStereoP,int width,int height);
    private native void nativeOnDrawFrame(long glRendererStereoP);
    private native void nativeOnVideoRatioChanged(long glRendererStereoP,int videoW,int videoH);
    private native void nativeUpdateHeadsetParams(long nativePointer,float screen_width_meters,
                                                  float screen_height_meters,
                                                  float screen_to_lens_distance,
                                                  float inter_lens_distance,
                                                  int vertical_alignment,
                                                  float tray_to_lens_distance,
                                                  float[] device_fov_left,
                                                  float[] radial_distortion_params,
                                                  int screenWidthP,int screenHeightP);

    private final Context mContext;
    // Opaque native pointer to the native GLRStereoNormal instance.
    private final long nativeGLRendererStereo;
    private SurfaceTexture mSurfaceTexture;
    private MVideoPlayer mVideoPlayer;
    private final TelemetryReceiver telemetryReceiver;


    public GLRStereoNormal(final Context activityContext,final TelemetryReceiver telemetryReceiver, long gvrApiNativeContext){
        mContext=activityContext;
        this.telemetryReceiver=telemetryReceiver;
        nativeGLRendererStereo=nativeConstruct(activityContext,telemetryReceiver.getNativeInstance(),
                gvrApiNativeContext, VideoNative.videoMode(mContext));

        GvrView view=new GvrView(activityContext);
        GvrViewerParams params=view.getGvrViewerParams();
        float[] fov=new float[4];
        fov[0]=params.getLeftEyeMaxFov().getLeft();
        fov[1]=params.getLeftEyeMaxFov().getRight();
        fov[2]=params.getLeftEyeMaxFov().getBottom();
        fov[3]=params.getLeftEyeMaxFov().getTop();
        float[] kN=params.getDistortion().getCoefficients();
        nativeUpdateHeadsetParams(nativeGLRendererStereo,view.getScreenParams().getWidthMeters(),view.getScreenParams().getHeightMeters(),
                params.getScreenToLensDistance(),params.getInterLensDistance(),params.getVerticalAlignment().ordinal(),params.getVerticalDistanceToLensCenter(),
                fov,kN,view.getScreenParams().getWidth(),view.getScreenParams().getHeight());
        view.shutdown();
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        final int mGLTextureVideo = videoTexture[0];
        mSurfaceTexture = new SurfaceTexture(mGLTextureVideo,false);
        nativeOnSurfaceCreated(nativeGLRendererStereo,mGLTextureVideo,mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeGLRendererStereo,width,height);
        if(SJ.DisableVSYNC(mContext)){
            MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
            MyEGLConfigChooser.setEglSurfaceAttrib(EGL_ANDROID_front_buffer_auto_refresh,EGL14.EGL_TRUE);
            EGL14.eglSwapBuffers(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
        }
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        startVideoPlayerIfNotAlreadyRunning();
        if(SJ.Disable60FPSLock(mContext)){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
        }
        if(mSurfaceTexture!=null){
            mSurfaceTexture.updateTexImage();
        }
        if(SJ.Disable60FPSLock(mContext)){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
        }
        nativeOnDrawFrame(nativeGLRendererStereo);
        //EGL14.eglSwapBuffers(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
    }


    public void onPause(){
        System.out.println("onPause()");
        if(mVideoPlayer!=null){
            mVideoPlayer.stop();
            mVideoPlayer=null;
        }
    }

    private void startVideoPlayerIfNotAlreadyRunning(){
        if(mVideoPlayer==null){
            Surface mVideoSurface=new Surface(mSurfaceTexture);
            mVideoPlayer=new MVideoPlayer(mContext,mVideoSurface,this);
            mVideoPlayer.start();
        }
    }

    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {
        nativeOnVideoRatioChanged(nativeGLRendererStereo,videoW,videoH);
    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        telemetryReceiver.setDecodingInfo(decodingInfo.currentFPS,decodingInfo.currentKiloBitsPerSecond,decodingInfo.avgParsingTime_ms,decodingInfo.avgWaitForInputBTime_ms,
                decodingInfo.avgHWDecodingTime_ms);
    }


    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeGLRendererStereo);
        } finally {
            super.finalize();
        }
    }
}
