package constantin.fpv_vr.GLRenderer;

import android.content.Context;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.telemetry.core.TelemetryReceiver;

/**
 * Renders OSD only
 */

public class GLRMono360 implements GLSurfaceView.Renderer {
    static {
        System.loadLibrary("GLRMono360");
    }
    private native long nativeConstruct(Context context,long telemetryReceiver);
    private native void nativeDelete(long glRendererMonoP);
    private native void nativeOnSurfaceCreated(long glRendererMonoP,Context androidContext);
    private native void nativeOnSurfaceChanged(long glRendererMonoP,int width,int height);
    private native void nativeOnDrawFrame(long glRendererMonoP);

    private final long nativeGLRendererMono;
    private final Context mContext;

    public GLRMono360(final Context context, final TelemetryReceiver telemetryReceiver){
        mContext=context;
        nativeGLRendererMono=nativeConstruct(context,telemetryReceiver.getNativeInstance());
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(nativeGLRendererMono,mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeGLRendererMono,width,height);
        //MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
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
