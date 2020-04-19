package constantin.fpv_vr.play_stereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.opengl.GLSurfaceView;
import android.os.Bundle;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.settings.SJ;
import constantin.fpv_vr.djiintegration.xdji.DJITelemetryReceiver;
import constantin.fpv_vr.djiintegration.xdji.DJIVideoPlayer;
import constantin.renderingx.core.VrActivity;
import constantin.renderingx.core.views.MyEGLConfigChooser;
import constantin.renderingx.core.views.MyGLSurfaceView;
import constantin.renderingx.core.views.MyVRLayout;

public class AStereoNormal extends VrActivity {
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyVRLayout mVrLayout = new MyVRLayout(this);
        MyGLSurfaceView mGLViewStereo = new MyGLSurfaceView(this);
        mGLViewStereo.setEGLContextClientVersion(2);
        mGLViewStereo.setEGLConfigChooser(new MyEGLConfigChooser(SJ.DisableVSYNC(this),SJ.MultiSampleAntiAliasing(this)));
        final DJIVideoPlayer videoPlayer=new DJIVideoPlayer(this);
        DJITelemetryReceiver telemetryReceiver = new DJITelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer());
        GLRStereoNormal mGLRStereoNormal = new GLRStereoNormal(this,videoPlayer.configure2(), telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext());
        videoPlayer.setIVideoParamsChanged(mGLRStereoNormal);
        mGLViewStereo.setRenderer(mGLRStereoNormal);
        mGLViewStereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLViewStereo.setPreserveEGLContextOnPause(true);
        mVrLayout.setPresentationView(mGLViewStereo);
        setContentView(mVrLayout);
        AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this, mVrLayout.getGvrApi());
    }

}