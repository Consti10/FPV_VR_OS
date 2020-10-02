package constantin.fpv_vr.play_stereo;

import android.content.Context;
import android.opengl.EGL14;
import android.opengl.EGLExt;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;


import constantin.fpv_vr.settings.SJ;
import constantin.renderingx.core.VSYNC;
import constantin.renderingx.core.xglview.GLContextSurfaceLess;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.player.VideoSettings;
import constantin.renderingx.core.vrsettings.ASettingsVR;


public class GLRStereoVR implements XGLSurfaceView.FullscreenRendererWithSurfaceTexture, IVideoParamsChanged, GLContextSurfaceLess.ISecondarySharedContext {
    static final String TAG="GLRStereoVR";
    static {
        System.loadLibrary("GLRStereoVR");
    }
    private native long nativeConstruct(Context context,long telemetryReceiver,long nativeGvrContext,int videoMode,long vsyncP);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnContextCreated(long glRendererStereoP,Context androidContext,int width,int height,SurfaceTextureHolder surfaceTextureHolder);
    private native void nativeOnDrawFrame(long glRendererStereoP);
    private native void nativeOnVideoRatioChanged(long glRendererStereoP,int videoW,int videoH);
    //
    private native void nativeOnSecondaryContextCreated(long nativePointer,final Context context);
    private native void nativeOnSecondaryContextDoWork(long nativePointer);

    private final Context mContext;
    // Opaque native pointer to the native GLRStereoVR instance.
    private final long nativeGLRendererStereo;
    private final TelemetryReceiver telemetryReceiver;
    private final boolean USE_PRESENTATION_TIME;

    public GLRStereoVR(final AppCompatActivity context, final TelemetryReceiver telemetryReceiver, long gvrApiNativeContext){
        mContext=context;
        this.telemetryReceiver=telemetryReceiver;
        //final VSYNC vsync=new VSYNC(context);
        nativeGLRendererStereo=nativeConstruct(context,telemetryReceiver.getNativeInstance(),
                gvrApiNativeContext, VideoSettings.videoMode(mContext),0);
        USE_PRESENTATION_TIME=ASettingsVR.getVR_RENDERING_MODE(context)==1;
    }

    @Override
    public void onContextCreated(int width, int height, SurfaceTextureHolder surfaceTextureHolder) {
        nativeOnContextCreated(nativeGLRendererStereo,mContext,width,height,surfaceTextureHolder);
    }

    @Override
    public void onDrawFrame() {
        if(USE_PRESENTATION_TIME){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
        }
        nativeOnDrawFrame(nativeGLRendererStereo);
        if(USE_PRESENTATION_TIME){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
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

    private void log(final String message){
        Log.d(TAG,message);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeGLRendererStereo);
        } finally {
            super.finalize();
        }
    }

    @Override
    public void onSecondaryContextCreated() {
        nativeOnSecondaryContextCreated(nativeGLRendererStereo,mContext);
    }

    @Override
    public void onSecondaryContextDoWork() {
        nativeOnSecondaryContextDoWork(nativeGLRendererStereo);
    }
}
