package constantin.fpv_vr.PlayStereo;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.view.Surface;

import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

import constantin.fpv_vr.MVideoPlayer;
import constantin.renderingx.core.GLESInfo.GLESInfo;
import constantin.renderingx.core.ViewSuperSync;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoNative.VideoNative;


/**
 * Description: see AStereoSuperSYNC
 */

public class GLRStereoSuperSync implements ViewSuperSync.IRendererSuperSync, IVideoParamsChanged {
    static {
        System.loadLibrary("GLRStereoSuperSync");
    }
    private native long nativeConstruct(Context context,long telemetryReceiver,long nativeGvrContext,boolean qcomTiledRenderingAvailable,
                                        boolean reusableSyncAvailable,int videoMode);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnSurfaceCreated(long glRendererStereoP,int videoTexture,Context androidContext);
    private native void nativeOnSurfaceChanged(long glRendererStereoP,int width,int height);
    private native void nativeEnterSuperSyncLoop(long glRendererStereoP,SurfaceTexture surfaceTexture,int exclusiveVRCore);
    private native void nativeExitSuperSyncLoop(long glRendererMonoP);
    private native void nativeOnVideoRatioChanged(long glRendererStereoP,int videoW,int videoH);
    private native void nativeDoFrame(long glRendererStereoP,long lastVsync);
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
    // Opaque native pointer to the native GLRStereoSuperSync instance.
    private final long nativeGLRSuperSync;
    private SurfaceTexture mSurfaceTexture;
    private MVideoPlayer mVideoPlayer;
    private int mGLTextureVideo;
    private final TelemetryReceiver telemetryReceiver;


    public GLRStereoSuperSync(final Context context,final TelemetryReceiver telemetryReceiver, long gvrApiNativeContext){
        mContext=context;
        this.telemetryReceiver=telemetryReceiver;
        final boolean qcomTiledRenderingAvailable= GLESInfo.isExtensionAvailable(context, GLESInfo.GL_QCOM_tiled_rendering);
        final boolean reusableSyncAvailable=GLESInfo.isExtensionAvailable(context,GLESInfo.EGL_KHR_reusable_sync);
        nativeGLRSuperSync=nativeConstruct(context,telemetryReceiver.getNativeInstance(),
                gvrApiNativeContext,qcomTiledRenderingAvailable,reusableSyncAvailable, VideoNative.videoMode(mContext));

        GvrView view=new GvrView(context);
        GvrViewerParams params=view.getGvrViewerParams();
        float[] fov=new float[4];
        fov[0]=params.getLeftEyeMaxFov().getLeft();
        fov[1]=params.getLeftEyeMaxFov().getRight();
        fov[2]=params.getLeftEyeMaxFov().getBottom();
        fov[3]=params.getLeftEyeMaxFov().getTop();
        float[] kN=params.getDistortion().getCoefficients();
        nativeUpdateHeadsetParams(nativeGLRSuperSync,view.getScreenParams().getWidthMeters(),view.getScreenParams().getHeightMeters(),
                params.getScreenToLensDistance(),params.getInterLensDistance(),params.getVerticalAlignment().ordinal(),params.getVerticalDistanceToLensCenter(),
                fov,kN,view.getScreenParams().getWidth(),view.getScreenParams().getHeight());
        view.shutdown();
    }

    @Override
    public void onSurfaceCreated() {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        mGLTextureVideo=videoTexture[0];
        nativeOnSurfaceCreated(nativeGLRSuperSync,videoTexture[0],mContext);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        nativeOnSurfaceChanged(nativeGLRSuperSync,width,height);
    }

    @Override
    public void enterSuperSyncLoop(final int exclusiveVRCore) {
        startVideoPlayerIfNotAlreadyRunning();

        nativeEnterSuperSyncLoop(nativeGLRSuperSync,mSurfaceTexture,exclusiveVRCore);

        stopVideoPlayerIfNotAlreadyRunning();
    }


    @Override
    public void requestExitSuperSyncLoop() {
        nativeExitSuperSyncLoop(nativeGLRSuperSync);
    }


    private void startVideoPlayerIfNotAlreadyRunning(){
        if(mVideoPlayer==null){
            mSurfaceTexture = new SurfaceTexture(mGLTextureVideo,false);
            Surface mVideoSurface=new Surface(mSurfaceTexture);
            mVideoPlayer=new MVideoPlayer(mContext,mVideoSurface,this);
            mVideoPlayer.start();
        }
    }

    private void stopVideoPlayerIfNotAlreadyRunning(){
        if(mVideoPlayer!=null){
            mVideoPlayer.stop();
            mVideoPlayer=null;
            mSurfaceTexture.release();
            mSurfaceTexture=null;
        }
    }

    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {
        nativeOnVideoRatioChanged(nativeGLRSuperSync,videoW,videoH);
    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        telemetryReceiver.setDecodingInfo(decodingInfo.currentFPS,decodingInfo.currentKiloBitsPerSecond,decodingInfo.avgParsingTime_ms,decodingInfo.avgWaitForInputBTime_ms,
                decodingInfo.avgHWDecodingTime_ms);
    }


    @Override
    public void setLastVSYNC(long lastVSYNC) {
        nativeDoFrame(nativeGLRSuperSync,lastVSYNC);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeGLRSuperSync);
        } finally {
            super.finalize();
        }
    }

}

