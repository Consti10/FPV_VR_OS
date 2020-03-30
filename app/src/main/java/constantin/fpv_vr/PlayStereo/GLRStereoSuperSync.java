package constantin.fpv_vr.PlayStereo;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;

import constantin.renderingx.core.GLESInfo.GLESInfo;
import constantin.renderingx.core.MyVrHeadsetParams;
import constantin.renderingx.core.ViewSuperSync;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.ISurfaceTextureAvailable;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer.VideoSettings;


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
    private final TelemetryReceiver telemetryReceiver;
    private final ISurfaceTextureAvailable iSurfaceTextureAvailable;

    public GLRStereoSuperSync(final Context context, final ISurfaceTextureAvailable iSurfaceTextureAvailable,final TelemetryReceiver telemetryReceiver, long gvrApiNativeContext){
        mContext=context;
        this.iSurfaceTextureAvailable=iSurfaceTextureAvailable;
        this.telemetryReceiver=telemetryReceiver;
        final boolean qcomTiledRenderingAvailable= GLESInfo.isExtensionAvailable(context, GLESInfo.GL_QCOM_tiled_rendering);
        final boolean reusableSyncAvailable=GLESInfo.isExtensionAvailable(context,GLESInfo.EGL_KHR_reusable_sync);
        nativeGLRSuperSync=nativeConstruct(context,telemetryReceiver.getNativeInstance(),
                gvrApiNativeContext,qcomTiledRenderingAvailable,reusableSyncAvailable, VideoSettings.videoMode(mContext));
        final MyVrHeadsetParams params=new MyVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeGLRSuperSync,params.ScreenWidthMeters,params.ScreenHeightMeters,
                params.ScreenToLensDistance,params.InterLensDistance,params.VerticalAlignment,params.VerticalDistanceToLensCenter,
                params.fov,params.kN,params.ScreenWidthPixels,params.ScreenHeightPixels);
    }

    @Override
    public void onSurfaceCreated() {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        final int mGLTextureVideo = videoTexture[0];
        mSurfaceTexture = new SurfaceTexture(mGLTextureVideo,false);
        iSurfaceTextureAvailable.onSurfaceTextureAvailable(mSurfaceTexture);
        nativeOnSurfaceCreated(nativeGLRSuperSync,videoTexture[0],mContext);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        nativeOnSurfaceChanged(nativeGLRSuperSync,width,height);
    }

    @Override
    public void enterSuperSyncLoop(final int exclusiveVRCore) {
        nativeEnterSuperSyncLoop(nativeGLRSuperSync,mSurfaceTexture,exclusiveVRCore);
    }

    @Override
    public void requestExitSuperSyncLoop() {
        nativeExitSuperSyncLoop(nativeGLRSuperSync);
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

