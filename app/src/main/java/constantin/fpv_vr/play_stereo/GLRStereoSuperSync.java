package constantin.fpv_vr.play_stereo;

import android.content.Context;
import android.graphics.SurfaceTexture;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.MVrHeadsetParams;
import constantin.renderingx.core.gles_info.GLESInfo;
import constantin.video.core.gl.ISurfaceAvailable;
import constantin.video.core.gl.VideoSurfaceHolder;
import constantin.renderingx.core.views.ViewSuperSync;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.video_player.VideoSettings;


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
    private native void nativeUpdateHeadsetParams(long nativePointer,MVrHeadsetParams params);

    private final Context mContext;
    // Opaque native pointer to the native GLRStereoSuperSync instance.
    private final long nativeGLRSuperSync;
    private final VideoSurfaceHolder videoSurfaceHolder;
    private final TelemetryReceiver telemetryReceiver;

    public GLRStereoSuperSync(final AppCompatActivity context, final ISurfaceAvailable iSurfaceAvailable, final TelemetryReceiver telemetryReceiver, long gvrApiNativeContext){
        mContext=context;
        this.telemetryReceiver=telemetryReceiver;
        videoSurfaceHolder=new VideoSurfaceHolder(context);
        videoSurfaceHolder.setCallBack(iSurfaceAvailable);
        final boolean qcomTiledRenderingAvailable= GLESInfo.isExtensionAvailable(context, GLESInfo.GL_QCOM_tiled_rendering);
        final boolean reusableSyncAvailable=GLESInfo.isExtensionAvailable(context,GLESInfo.EGL_KHR_reusable_sync);
        nativeGLRSuperSync=nativeConstruct(context,telemetryReceiver.getNativeInstance(),
                gvrApiNativeContext,qcomTiledRenderingAvailable,reusableSyncAvailable, VideoSettings.videoMode(mContext));
        final MVrHeadsetParams params=new MVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeGLRSuperSync,params);
    }

    @Override
    public void onSurfaceCreated() {
        videoSurfaceHolder.createSurfaceTextureGL();
        nativeOnSurfaceCreated(nativeGLRSuperSync,videoSurfaceHolder.getTextureId(),mContext);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        nativeOnSurfaceChanged(nativeGLRSuperSync,width,height);
    }

    @Override
    public void enterSuperSyncLoop(final int exclusiveVRCore) {
        nativeEnterSuperSyncLoop(nativeGLRSuperSync,videoSurfaceHolder.getSurfaceTexture(),exclusiveVRCore);
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

