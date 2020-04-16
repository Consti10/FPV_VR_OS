package constantin.fpv_vr.play_stereo;

import android.os.Bundle;
import android.view.KeyEvent;

import androidx.appcompat.app.AppCompatActivity;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.xdji.XTelemetryReceiver;
import constantin.fpv_vr.xdji.XVideoPlayerSurfaceTexture;
import constantin.renderingx.core.views.ViewSuperSync;

/*****************************************
 * Render Video & OSD Side by Side. Difference to AStereoNormal: Renders directly into the Front Buffer  (FB) for lower latency
 * Synchronisation with the VSYNC is done in cpp.
 * Pipeline h.264-->image on screen:
 * h.264 NALUs->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 **************************************** */

public class AStereoSuperSYNC extends AppCompatActivity{
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ViewSuperSync mViewSuperSync = new ViewSuperSync(this);
        XVideoPlayerSurfaceTexture videoPlayer=new XVideoPlayerSurfaceTexture(this);
        XTelemetryReceiver telemetryReceiver = new XTelemetryReceiver(this,videoPlayer.getExternalGroundRecorder());
        GLRStereoSuperSync mGLRStereoSuperSync = new GLRStereoSuperSync(this,videoPlayer, telemetryReceiver, mViewSuperSync.getGvrApi().getNativeGvrContext());
        videoPlayer.setIVideoParamsChanged(mGLRStereoSuperSync);
        mViewSuperSync.setRenderer(mGLRStereoSuperSync);
        setContentView(mViewSuperSync);
        AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this, mViewSuperSync.getGvrApi());
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        //Some VR headsets use a clamp to hold the phone in place. This clamp may press against the volume up/down buttons.
        //Here we effectively disable these 2 buttons
        if(event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_DOWN || event.getKeyCode()==KeyEvent.KEYCODE_VOLUME_UP){
            return true;
        }
        return super.dispatchKeyEvent(event);
    }

}


