package constantin.fpv_vr.PlayStereo;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.view.Surface;

import constantin.fpv_vr.MVideoPlayer;
import constantin.fpv_vr.Settings.VRSettingsHelper;
import constantin.renderingX.GLESInfo.GLESInfo;
import constantin.renderingX.ViewSuperSync;
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
    private native long nativeConstruct(Context context,float[] radialUndistortionData,long telemetryReceiver,long nativeGvrContext,boolean qcomTiledRenderingAvailable,boolean reusableSyncAvailable,boolean is360);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnSurfaceCreated(long glRendererStereoP,int videoTexture,Context androidContext);
    private native void nativeOnSurfaceChanged(long glRendererStereoP,int width,int height);
    private native void nativeEnterSuperSyncLoop(long glRendererStereoP,SurfaceTexture surfaceTexture,int exclusiveVRCore);
    private native void nativeExitSuperSyncLoop(long glRendererMonoP);
    private native void nativeOnVideoRatioChanged(long glRendererStereoP,int videoW,int videoH);
    private native void nativeDoFrame(long glRendererStereoP,long lastVsync);

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
        nativeGLRSuperSync=nativeConstruct(context, VRSettingsHelper.getUndistortionCoeficients(context),telemetryReceiver.getNativeInstance(),
                gvrApiNativeContext,qcomTiledRenderingAvailable,reusableSyncAvailable, VideoNative.video360(mContext));
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

