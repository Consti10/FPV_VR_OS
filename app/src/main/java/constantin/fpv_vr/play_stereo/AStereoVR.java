package constantin.fpv_vr.play_stereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.os.Bundle;
import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.fpv_vr.djiintegration.TelemetryReceiverDJI;
import constantin.fpv_vr.djiintegration.VideoPlayerDJI;
import constantin.fpv_vr.settings.SJ;
import constantin.renderingx.core.views.VrActivity;
import constantin.renderingx.core.views.VrView;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.uvcintegration.UVCPlayer;
import constantin.video.core.player.VideoPlayer;

public class AStereoVR extends VrActivity {
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        VrView vrView=new VrView(this);
        //vrView.enableSuperSync();

        final GLRStereoVR mGLRStereoVR;
        if(SJ.getConnectionType(this)== AConnect.CONNECTION_TYPE_UVC){
            final UVCPlayer uvcPlayer=new UVCPlayer(this);
            final TelemetryReceiver telemetryReceiver=new TelemetryReceiver(this,0,0);
            mGLRStereoVR = new GLRStereoVR(this, telemetryReceiver,vrView.getGvrApi().getNativeGvrContext());
            uvcPlayer.setIVideoParamsChanged(mGLRStereoVR);
            vrView.getPresentationView().setRenderer(mGLRStereoVR,uvcPlayer.configure2());
            vrView.getPresentationView().setmISecondaryContext(mGLRStereoVR);
        }else{
            final VideoPlayer videoPlayer= DJIApplication.isDJIEnabled(this) ?
                    new VideoPlayerDJI(this):
                    new VideoPlayer(this);
            final TelemetryReceiver telemetryReceiver= DJIApplication.isDJIEnabled(this) ?
                    new TelemetryReceiverDJI(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer()):
                    new TelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer());
            mGLRStereoVR = new GLRStereoVR(this, telemetryReceiver, vrView.getGvrApi().getNativeGvrContext());
            videoPlayer.setIVideoParamsChanged(mGLRStereoVR);
            vrView.getPresentationView().setRenderer(mGLRStereoVR,videoPlayer.configure2());
            vrView.getPresentationView().setmISecondaryContext(mGLRStereoVR);
        }
        setContentView(vrView);
        AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this,vrView.getGvrApi());
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