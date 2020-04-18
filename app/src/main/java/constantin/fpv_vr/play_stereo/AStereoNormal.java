package constantin.fpv_vr.play_stereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.KeyEvent;

import androidx.appcompat.app.AppCompatActivity;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.settings.SJ;
import constantin.fpv_vr.xdji.XTelemetryReceiver;
import constantin.fpv_vr.xdji.XVideoPlayer;
import constantin.renderingx.core.views.MyEGLConfigChooser;
import constantin.renderingx.core.views.MyGLSurfaceView;
import constantin.renderingx.core.views.MyVRLayout;

public class AStereoNormal extends AppCompatActivity{
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyVRLayout mVrLayout = new MyVRLayout(this);
        MyGLSurfaceView mGLViewStereo = new MyGLSurfaceView(this);
        mGLViewStereo.setEGLContextClientVersion(2);
        mGLViewStereo.setEGLConfigChooser(new MyEGLConfigChooser(SJ.DisableVSYNC(this),SJ.MultiSampleAntiAliasing(this)));
        final XVideoPlayer videoPlayer=new XVideoPlayer(this);
        XTelemetryReceiver telemetryReceiver = new XTelemetryReceiver(this,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFileReader());
        GLRStereoNormal mGLRStereoNormal = new GLRStereoNormal(this,videoPlayer, telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext());
        videoPlayer.setIVideoParamsChanged(mGLRStereoNormal);
        mGLViewStereo.setRenderer(mGLRStereoNormal);
        mGLViewStereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLViewStereo.setPreserveEGLContextOnPause(true);
        mVrLayout.setPresentationView(mGLViewStereo);
        setContentView(mVrLayout);
        AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this, mVrLayout.getGvrApi());
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