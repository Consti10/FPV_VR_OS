package constantin.fpv_vr.PlayStereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import android.view.KeyEvent;
import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.XDJI.DJIVideoPlayerSurfaceTexture;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyGLSurfaceView;
import constantin.renderingx.core.MyVRLayout;
import constantin.telemetry.core.TelemetryReceiver;

public class AStereoNormal extends AppCompatActivity{
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here
    private MyVRLayout mVrLayout;
    private MyGLSurfaceView mGLViewStereo;

    private GLRStereoNormal mGLRStereoNormal;
    private AirHeadTrackingSender airHeadTrackingSender;
    private TelemetryReceiver telemetryReceiver;
    //private VideoPlayerSurfaceTexture mVideoPlayer;
    private DJIVideoPlayerSurfaceTexture mVideoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mVrLayout =new MyVRLayout(this,false);
        mGLViewStereo=new MyGLSurfaceView(this,this);
        mGLViewStereo.setEGLContextClientVersion(2);
        mGLViewStereo.setEGLConfigChooser(new MyEGLConfigChooser(SJ.DisableVSYNC(this),SJ.MultiSampleAntiAliasing(this)));
        //mVideoPlayer=new VideoPlayerSurfaceTexture(this);
        mVideoPlayer=new DJIVideoPlayerSurfaceTexture(this);
        telemetryReceiver=new TelemetryReceiver(this,mVideoPlayer.GetExternalGroundRecorder());
        mGLRStereoNormal = new GLRStereoNormal(this,mVideoPlayer,telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext());
        mVideoPlayer.setIVideoParamsChanged(mGLRStereoNormal);

        mGLViewStereo.setRenderer(mGLRStereoNormal);
        mGLViewStereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLViewStereo.setPreserveEGLContextOnPause(true);
        mVrLayout.setPresentationView(mGLViewStereo);
        setContentView(mVrLayout);
        airHeadTrackingSender=AirHeadTrackingSender.createIfEnabled(this,mVrLayout.getGvrApi());
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