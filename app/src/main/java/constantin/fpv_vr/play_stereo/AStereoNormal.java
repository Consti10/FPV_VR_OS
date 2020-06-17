package constantin.fpv_vr.play_stereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.graphics.Canvas;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.text.Spannable;
import android.widget.TextView;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.fpv_vr.djiintegration.DJITelemetryReceiver;
import constantin.fpv_vr.djiintegration.DJIVideoPlayer;
import constantin.fpv_vr.settings.SJ;
import constantin.renderingx.core.VrActivity;
import constantin.renderingx.core.views.DebugEGLContextFactory;
import constantin.renderingx.core.views.MyEGLConfigChooser;
import constantin.renderingx.core.views.MyGLSurfaceView;
import constantin.renderingx.core.views.MyVRLayout;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.test.UVCPlayer;
import constantin.video.core.video_player.VideoPlayer;

public class AStereoNormal extends VrActivity {
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyVRLayout mVrLayout = new MyVRLayout(this);
        MyGLSurfaceView mGLViewStereo = new MyGLSurfaceView(this);

        mGLViewStereo.setEGLContextClientVersion(2);
        mGLViewStereo.setEGLContextFactory(new DebugEGLContextFactory());
        mGLViewStereo.setEGLConfigChooser(new MyEGLConfigChooser(SJ.DisableVSYNC(this),SJ.MultiSampleAntiAliasing(this)));
        final GLRStereoNormal mGLRStereoNormal;
        if(SJ.getConnectionType(this)== AConnect.CONNECTION_TYPE_UVC){
            final UVCPlayer uvcPlayer=new UVCPlayer(this);
            final TelemetryReceiver telemetryReceiver=new TelemetryReceiver(this,0,0);
            mGLRStereoNormal = new GLRStereoNormal(this,uvcPlayer.configure2(), telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext(),mGLViewStereo);
            uvcPlayer.setIVideoParamsChanged(mGLRStereoNormal);
        }else{
            final VideoPlayer videoPlayer= DJIApplication.isDJIEnabled(this) ?
                    new DJIVideoPlayer(this):
                    new VideoPlayer(this);
            final TelemetryReceiver telemetryReceiver= DJIApplication.isDJIEnabled(this) ?
                    new DJITelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer()):
                    new TelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer());
            mGLRStereoNormal = new GLRStereoNormal(this,videoPlayer.configure2(), telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext(),mGLViewStereo);
            videoPlayer.setIVideoParamsChanged(mGLRStereoNormal);
        }
        mGLViewStereo.setRenderer(mGLRStereoNormal);
        mGLViewStereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLViewStereo.setPreserveEGLContextOnPause(true);
        mVrLayout.setPresentationView(mGLViewStereo);
        //mVrLayout.setVrOverlayEnabled(false);
        setContentView(mVrLayout);
        AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this, mVrLayout.getGvrApi());
    }

    @Override
    protected void onResume(){
        super.onResume();
        //Debug.startMethodTracing();
    }

    @Override
    protected void onPause(){
        super.onPause();
        //Debug.stopMethodTracing();
    }

}