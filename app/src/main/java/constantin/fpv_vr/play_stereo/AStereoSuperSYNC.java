package constantin.fpv_vr.play_stereo;

import android.os.Bundle;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.fpv_vr.djiintegration.DJITelemetryReceiver;
import constantin.fpv_vr.djiintegration.DJIVideoPlayer;
import constantin.fpv_vr.settings.SJ;
import constantin.renderingx.core.VrActivity;
import constantin.renderingx.core.views.ViewSuperSync;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.test.UVCPlayer;
import constantin.video.core.video_player.VideoPlayer;

/*****************************************
 * Render Video & OSD Side by Side. Difference to AStereoNormal: Renders directly into the Front Buffer  (FB) for lower latency
 * Synchronisation with the VSYNC is done in cpp.
 * Pipeline h.264-->image on screen:
 * h.264 NALUs->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 **************************************** */

public class AStereoSuperSYNC extends VrActivity {
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ViewSuperSync mViewSuperSync = new ViewSuperSync(this);
        final GLRStereoSuperSync mGLRStereoSuperSync;
        if(SJ.getConnectionType(this)== AConnect.CONNECTION_TYPE_UVC){
            final UVCPlayer uvcPlayer=new UVCPlayer(this);
            final TelemetryReceiver telemetryReceiver=new TelemetryReceiver(this,0,0);
            mGLRStereoSuperSync = new GLRStereoSuperSync(this,uvcPlayer.configure2(), telemetryReceiver, mViewSuperSync.getGvrApi().getNativeGvrContext());
            uvcPlayer.setIVideoParamsChanged(mGLRStereoSuperSync);
        }else{
            final VideoPlayer videoPlayer= DJIApplication.isDJIEnabled(this) ?
                    new DJIVideoPlayer(this):
                    new VideoPlayer(this);
            final TelemetryReceiver telemetryReceiver= DJIApplication.isDJIEnabled(this) ?
                    new DJITelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer()):
                    new TelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer());
            mGLRStereoSuperSync = new GLRStereoSuperSync(this,videoPlayer.configure2(), telemetryReceiver, mViewSuperSync.getGvrApi().getNativeGvrContext());
            videoPlayer.setIVideoParamsChanged(mGLRStereoSuperSync);
        }
        mViewSuperSync.setRenderer(mGLRStereoSuperSync);
        setContentView(mViewSuperSync);
        AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this, mViewSuperSync.getGvrApi());
    }

}


