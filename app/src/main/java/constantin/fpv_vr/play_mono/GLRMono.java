package constantin.fpv_vr.play_mono;

import android.content.Context;
import android.opengl.EGL14;
import android.opengl.GLSurfaceView;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;


import constantin.fpv_vr.R;
import constantin.renderingx.core.xglview.SurfaceTextureHolder;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.telemetry.core.TelemetryReceiver;

/*
 * Renders OSD and/or video in monoscopic view
 */
@SuppressWarnings("WeakerAccess")
public class GLRMono implements XGLSurfaceView.FullscreenRendererWithSurfaceTexture,XGLSurfaceView.FullscreenRenderer {
    static {
        System.loadLibrary("GLRMono");
    }
    native long nativeConstruct(Context context,long nativeTelemetryReceiver,long nativeGvrContext,int videoMode,boolean enableOSD);
    native void nativeDelete(long glRendererMonoP);
    native void nativeOnSurfaceCreated(long glRendererMonoP,Context androidContext,SurfaceTextureHolder surfaceTextureHolder);
    native void nativeOnSurfaceChanged(long glRendererMonoP,int width,int height,float optionalVideo360FOV);
    native void nativeOnDrawFrame(long glRendererMonoP);
    native void nativeSetHomeOrientation360(long glRendererMonoP);
    private native void nativeOnVideoRatioChanged(long glRenderer,int videoW,int videoH);

    private final long nativeGLRendererMono;
    private final Context mContext;
    //Optional, only when playing video that cannot be displayed by a 'normal' android surface
    //(e.g. 360° video)
    private final boolean disableVSYNC;

    public GLRMono(final AppCompatActivity context,final TelemetryReceiver telemetryReceiver, GvrApi gvrApi, final int videoMode, final boolean renderOSD,
                   final boolean disableVSYNC){
        mContext=context;
        this.disableVSYNC=disableVSYNC;
        nativeGLRendererMono=nativeConstruct(context,telemetryReceiver.getNativeInstance(),gvrApi.getNativeGvrContext(),videoMode,renderOSD);
    }

    public void setVideoRatio(int videoW, int videoH) {
        nativeOnVideoRatioChanged(nativeGLRendererMono,videoW,videoH);
    }

    public void setHomeOrientation(){
        nativeSetHomeOrientation360(nativeGLRendererMono);
    }

    @Override
    public void onContextCreated(int screenWidth, int screenHeight) {
        nativeOnSurfaceCreated(nativeGLRendererMono,mContext,null);
        final float video360FOV=mContext.getSharedPreferences("pref_video",Context.MODE_PRIVATE).getFloat(mContext.getString(R.string.VS_360_VIDEO_FOV),50);
        nativeOnSurfaceChanged(nativeGLRendererMono,screenWidth,screenHeight,video360FOV);
    }

    @Override
    public void onContextCreated(int screenWidth, int screenHeight, SurfaceTextureHolder surfaceTextureHolder) {
        nativeOnSurfaceCreated(nativeGLRendererMono,mContext,surfaceTextureHolder);
        final float video360FOV=mContext.getSharedPreferences("pref_video",Context.MODE_PRIVATE).getFloat(mContext.getString(R.string.VS_360_VIDEO_FOV),50);
        nativeOnSurfaceChanged(nativeGLRendererMono,screenWidth,screenHeight,video360FOV);
    }

    @Override
    public void onDrawFrame() {
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
}
