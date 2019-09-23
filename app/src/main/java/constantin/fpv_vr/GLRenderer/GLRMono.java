package constantin.fpv_vr.GLRenderer;

import android.content.Context;


class GLRMono {
    static final int VIDEO_MODE_NONE=0;
    static final int VIDEO_MODE_STEREO=1;
    static final int VIDEO_MODE_360=2;
    static {
        System.loadLibrary("GLRMono");
    }
    native long nativeConstruct(Context context,long nativeTelemetryReceiver,long nativeGvrContext,int videoMode,boolean enableOSD);
    native void nativeDelete(long glRendererMonoP);
    native void nativeOnSurfaceCreated(long glRendererMonoP,Context androidContext,int optionalVideoTexture);
    native void nativeOnSurfaceChanged(long glRendererMonoP,int width,int height,float optionalVideo360FOV);
    native void nativeOnDrawFrame(long glRendererMonoP);
    native void nativeSetHomeOrientation360(long glRendererMonoP);
}
