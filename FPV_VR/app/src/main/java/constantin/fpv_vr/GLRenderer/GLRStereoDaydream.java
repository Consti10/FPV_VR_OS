package constantin.fpv_vr.GLRenderer;

import android.content.Context;
import android.opengl.GLSurfaceView;

import com.google.vr.ndk.base.GvrApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.telemetry.core.TelemetryReceiver;


public class GLRStereoDaydream implements GLSurfaceView.Renderer{
    static {
        System.loadLibrary("GLRStereoDaydream");
    }
    private native long nativeConstruct(Context context,long telemetryReceiver,long nativeGvrContext,int videoSurfaceID);
    private native void nativeDelete(long glRendererP);
    private native void nativeOnSurfaceCreated(long glRendererP,float fovY_full,float ipd_full,Context androidContext);
    private native void nativeOnSurfaceChanged(long glRendererP,int width,int height);
    private native void nativeOnDrawFrame(long glRendererP);

    // Opaque native pointer to the native GLRStereoNormal instance.
    private final long nativeGLRendererDaydream;
    private final Context mContext;

    public GLRStereoDaydream(Context context, final TelemetryReceiver telemetryReceiver, GvrApi api, int videoSurfaceID){
        mContext=context;
        nativeGLRendererDaydream=nativeConstruct(context,telemetryReceiver.getNativeInstance(),
                api.getNativeGvrContext(),videoSurfaceID);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(nativeGLRendererDaydream,0,0,mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeGLRendererDaydream,width,height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        nativeOnDrawFrame(nativeGLRendererDaydream);
    }


    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeGLRendererDaydream);
        } finally {
            super.finalize();
        }
    }
}
