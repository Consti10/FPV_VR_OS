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
import android.view.View;
import android.widget.TextView;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.fpv_vr.djiintegration.DJITelemetryReceiver;
import constantin.fpv_vr.djiintegration.DJIVideoPlayer;
import constantin.fpv_vr.settings.SJ;
import constantin.renderingx.core.VrActivity;
import constantin.renderingx.core.views.VrView;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.test.UVCPlayer;
import constantin.video.core.video_player.VideoPlayer;

public class AStereoNormal extends VrActivity {
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        VrView vrView=new VrView(this);

        final GLRStereoNormal mGLRStereoNormal;
        if(SJ.getConnectionType(this)== AConnect.CONNECTION_TYPE_UVC){
            final UVCPlayer uvcPlayer=new UVCPlayer(this);
            final TelemetryReceiver telemetryReceiver=new TelemetryReceiver(this,0,0);
            mGLRStereoNormal = new GLRStereoNormal(this, telemetryReceiver,vrView.getGvrApi().getNativeGvrContext());
            uvcPlayer.setIVideoParamsChanged(mGLRStereoNormal);
            vrView.setRenderer(mGLRStereoNormal,uvcPlayer.configure2());
            vrView.setmISecondaryContext(mGLRStereoNormal);
        }else{
            final VideoPlayer videoPlayer= DJIApplication.isDJIEnabled(this) ?
                    new DJIVideoPlayer(this):
                    new VideoPlayer(this);
            final TelemetryReceiver telemetryReceiver= DJIApplication.isDJIEnabled(this) ?
                    new DJITelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer()):
                    new TelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer());
            mGLRStereoNormal = new GLRStereoNormal(this, telemetryReceiver, vrView.getGvrApi().getNativeGvrContext());
            videoPlayer.setIVideoParamsChanged(mGLRStereoNormal);
            vrView.setRenderer(mGLRStereoNormal,videoPlayer.configure2());
            vrView.setmISecondaryContext(mGLRStereoNormal);
        }
        //mVrLayout.setVrOverlayEnabled(false);
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