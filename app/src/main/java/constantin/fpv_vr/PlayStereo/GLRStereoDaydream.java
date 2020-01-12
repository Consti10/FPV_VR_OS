package constantin.fpv_vr.PlayStereo;

import android.content.Context;
import android.opengl.GLSurfaceView;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

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

    private native void nativeUpdateHeadsetParams(long nativePointer,float screen_width_meters,
                                                  float screen_height_meters,
                                                  float screen_to_lens_distance,
                                                  float inter_lens_distance,
                                                  int vertical_alignment,
                                                  float tray_to_lens_distance,
                                                  float[] device_fov_left,
                                                  float[] radial_distortion_params,
                                                  int screenWidthP,int screenHeightP);

    // Opaque native pointer to the native GLRStereoNormal instance.
    private final long nativeGLRendererDaydream;
    private final Context mContext;

    public GLRStereoDaydream(Context context, final TelemetryReceiver telemetryReceiver, GvrApi api, int videoSurfaceID){
        mContext=context;
        GvrView view=new GvrView(context);
        final GvrViewerParams params=view.getGvrViewerParams();

        nativeGLRendererDaydream=nativeConstruct(context,telemetryReceiver.getNativeInstance(),
                api.getNativeGvrContext(),videoSurfaceID);


        float[] fov=new float[4];
        fov[0]=params.getLeftEyeMaxFov().getLeft();
        fov[1]=params.getLeftEyeMaxFov().getRight();
        fov[2]=params.getLeftEyeMaxFov().getBottom();
        fov[3]=params.getLeftEyeMaxFov().getTop();

        float[] kN=params.getDistortion().getCoefficients();

        nativeUpdateHeadsetParams(nativeGLRendererDaydream,view.getScreenParams().getWidthMeters(),view.getScreenParams().getHeightMeters(),
                params.getScreenToLensDistance(),params.getInterLensDistance(),params.getVerticalAlignment().ordinal(),params.getVerticalDistanceToLensCenter(),
                fov,kN,view.getScreenParams().getWidth(),view.getScreenParams().getHeight());
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
