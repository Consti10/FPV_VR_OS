package constantin.fpv_vr.PlayStereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;

import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.ndk.base.BufferViewport;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyGLSurfaceView;
import constantin.renderingx.core.MyVRLayout;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.VideoPlayerSurfaceTexture;

public class AStereoNormal extends AppCompatActivity{
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here
    private MyVRLayout mVrLayout;
    private MyGLSurfaceView mGLViewStereo;

    private GLRStereoNormal mGLRStereoNormal;
    private AirHeadTrackingSender airHeadTrackingSender;
    private TelemetryReceiver telemetryReceiver;
    private VideoPlayerSurfaceTexture mVideoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mVrLayout =new MyVRLayout(this,false);
        mGLViewStereo=new MyGLSurfaceView(this,this);
        mGLViewStereo.setEGLContextClientVersion(2);
        mGLViewStereo.setEGLConfigChooser(new MyEGLConfigChooser(SJ.DisableVSYNC(this),SJ.MultiSampleAntiAliasing(this)));
        //mGLViewStereo.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory(true));
        telemetryReceiver=new TelemetryReceiver(this);
        mVideoPlayer=new VideoPlayerSurfaceTexture(this);
        mGLRStereoNormal = new GLRStereoNormal(this,mVideoPlayer,telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext());
        mVideoPlayer.setIVideoParamsChanged(mGLRStereoNormal);

        mGLViewStereo.setRenderer(mGLRStereoNormal);
        mGLViewStereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLViewStereo.setPreserveEGLContextOnPause(true);
        mVrLayout.setPresentationView(mGLViewStereo);
        setContentView(mVrLayout);
        airHeadTrackingSender=new AirHeadTrackingSender(this, mVrLayout.getGvrApi());
    }

}