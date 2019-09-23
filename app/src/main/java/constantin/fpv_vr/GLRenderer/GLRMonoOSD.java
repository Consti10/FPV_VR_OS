package constantin.fpv_vr.GLRenderer;

import android.content.Context;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;

/**
 * Renders OSD only
 */

public class GLRMonoOSD extends GLRMono implements GLSurfaceView.Renderer, IVideoParamsChanged {

    private final long nativeGLRendererMono;
    private final Context mContext;
    private final TelemetryReceiver telemetryReceiver;

    public GLRMonoOSD(final Context context, final TelemetryReceiver telemetryReceiver){
        mContext=context;
        this.telemetryReceiver=telemetryReceiver;
        nativeGLRendererMono=nativeConstruct(context,telemetryReceiver.getNativeInstance(),0,VIDEO_MODE_NONE,true);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(nativeGLRendererMono,mContext,0);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeGLRendererMono,width,height,0);
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

    @Override
    public void onVideoRatioChanged(int videoW, int videoH) { }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        if(telemetryReceiver!=null){
            telemetryReceiver.setDecodingInfo(decodingInfo.currentFPS,decodingInfo.currentKiloBitsPerSecond,decodingInfo.avgParsingTime_ms,decodingInfo.avgWaitForInputBTime_ms,
                    decodingInfo.avgHWDecodingTime_ms);
        }
    }
}
