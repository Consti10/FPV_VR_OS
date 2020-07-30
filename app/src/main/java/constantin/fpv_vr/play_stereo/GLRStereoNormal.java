package constantin.fpv_vr.play_stereo;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLExt;
import android.opengl.GLSurfaceView;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.fpv_vr.settings.SJ;
import constantin.renderingx.core.MVrHeadsetParams;
import constantin.renderingx.core.views.MyEGLConfigChooser;
import constantin.renderingx.core.xglview.GLContextSurfaceLess;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.gl.ISurfaceAvailable;
import constantin.video.core.gl.VideoSurfaceHolder;
import constantin.video.core.video_player.VideoSettings;

import static constantin.renderingx.core.views.MyEGLConfigChooser.EGL_ANDROID_front_buffer_auto_refresh;

/** Open GL renderer stereo in normal mode (normal==no SuperSync or daydream, but possibly VSYNC disabled)
 * Description: see AStereoNormal
 */

public class GLRStereoNormal implements XGLSurfaceView.FullscreenRenderer, IVideoParamsChanged, GLContextSurfaceLess.SecondarySharedContext {
    static final String TAG="GLRStereoNormal";
    static {
        System.loadLibrary("GLRStereoNormal");
    }
    private native long nativeConstruct(Context context,long telemetryReceiver,long nativeGvrContext,int videoMode);
    private native void nativeDelete(long glRendererStereoP);
    private native void nativeOnContextCreated(long glRendererStereoP,Context androidContext,SurfaceTexture videoSurfaceTexture,int videoSurfaceTextureId,int width,int height);
    private native void nativeOnDrawFrame(long glRendererStereoP);
    private native void nativeOnVideoRatioChanged(long glRendererStereoP,int videoW,int videoH);
    private native void nativeUpdateHeadsetParams(long nativePointer,MVrHeadsetParams params);
    //
    private native void nativeOnSecondaryContextCreated(long nativePointer,final Context context);
    private native void nativeOnSecondaryContextDoWork(long nativePointer);

    private final Context mContext;
    // Opaque native pointer to the native GLRStereoNormal instance.
    private final long nativeGLRendererStereo;
    private VideoSurfaceHolder videoSurfaceHolder;
    private final TelemetryReceiver telemetryReceiver;

    public GLRStereoNormal(final AppCompatActivity context, final ISurfaceAvailable iSurfaceAvailable, final TelemetryReceiver telemetryReceiver, long gvrApiNativeContext,final XGLSurfaceView view){
        mContext=context;
        this.telemetryReceiver=telemetryReceiver;
        videoSurfaceHolder=new VideoSurfaceHolder(context);
        videoSurfaceHolder.setCallBack(iSurfaceAvailable);
        nativeGLRendererStereo=nativeConstruct(context,telemetryReceiver.getNativeInstance(),
                gvrApiNativeContext, VideoSettings.videoMode(mContext));
        final MVrHeadsetParams params=new MVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeGLRendererStereo,params);
    }

    @Override
    public void onContextCreated(int width, int height) {
        videoSurfaceHolder.createSurfaceTextureGL();
        nativeOnContextCreated(nativeGLRendererStereo,mContext,videoSurfaceHolder.getSurfaceTexture(),videoSurfaceHolder.getTextureId(),width,height);

    }

    @Override
    public void onDrawFrame() {
        if(SJ.Disable60FPSLock(mContext)){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
        }
        if(SJ.Disable60FPSLock(mContext)){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
        }
        nativeOnDrawFrame(nativeGLRendererStereo);
        //EGL14.eglSwapBuffers(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
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
