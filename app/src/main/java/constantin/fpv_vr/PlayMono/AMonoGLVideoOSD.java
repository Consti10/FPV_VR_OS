package constantin.fpv_vr.PlayMono;

import androidx.appcompat.app.AppCompatActivity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.XDJI.DJITelemetryReceiver;
import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyEGLWindowSurfaceFactory;
import constantin.renderingx.core.MyGLSurfaceView;
import constantin.renderingx.core.MyVRLayout;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.External.AspectFrameLayout;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer.VideoPlayer;
import constantin.video.core.VideoPlayer.VideoSettings;
import constantin.video.core.VideoPlayerSurfaceHolder;
import constantin.video.core.VideoPlayerSurfaceTexture;


/*****************************************************************
 * Play video blended with OSD in a Mono window, but without using the HW composer for video
 * 360° or stereo video needs to be rendered with OpenGL regardless weather the user wants to see the OSD or not
 * OSD can be fully disabled
 ***************************************************************** */

public class AMonoGLVideoOSD extends AppCompatActivity {
    private static final String TAG="AMonoGLVideoOSD";
    private MyGLSurfaceView mGLView;
    private GLRMono mGLRenderer;
    //private TelemetryReceiver telemetryReceiver;
    private DJITelemetryReceiver telemetryReceiver;
    private MyVRLayout myVRLayout;
    public static final String EXTRA_RENDER_OSD ="EXTRA_RENDER_OSD"; //boolean weather ENABLE_OSD should be enabled
    private VideoPlayerSurfaceTexture mVideoPlayer;
    private AirHeadTrackingSender airHeadTrackingSender;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final boolean renderOSD=getIntent().getBooleanExtra(EXTRA_RENDER_OSD,true);
        MyVRLayout.enableSustainedPerformanceIfPossible(this);
        mGLView = new MyGLSurfaceView(this,this);
        mGLView.setEGLContextClientVersion(2);
        //for now do not differentiate
        final boolean disableVSYNC = SJ.DisableVSYNC(this);
        //do not use MSAA in mono mode
        mGLView.setEGLConfigChooser(new MyEGLConfigChooser(disableVSYNC, 0,true));
        mGLView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());

        myVRLayout=new MyVRLayout(this);
        myVRLayout.setVrOverlayEnabled(false);
        myVRLayout.setPresentationView(mGLView);

        mVideoPlayer=new VideoPlayerSurfaceTexture(this);
        telemetryReceiver=new DJITelemetryReceiver(this,mVideoPlayer.GetExternalGroundRecorder());
        mGLRenderer =new GLRMono(this,mVideoPlayer,telemetryReceiver, myVRLayout.getGvrApi(),
                VideoSettings.videoMode(this),renderOSD, disableVSYNC);
        mVideoPlayer.setIVideoParamsChanged(mGLRenderer);
        mGLView.setRenderer(mGLRenderer);

        setContentView(myVRLayout);
        registerForContextMenu(myVRLayout);

        airHeadTrackingSender=AirHeadTrackingSender.createIfEnabled(this,myVRLayout.getGvrApi());
    }


    @Override
    protected void onResume() {
        super.onResume();
        //Log.d(TAG, "onResume");
        FullscreenHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        menu.setHeaderTitle("Options");
        getMenuInflater().inflate(R.menu.video360_context_menu, menu);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.option_set_home:
                mGLRenderer.setHomeOrientation();
                return true;
            case R.id.option_goto_home:
                //mGLRenderer14Mono360.goToHomeOrientation();
                GvrApi api= myVRLayout.getGvrApi();
                api.recenterTracking();
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }
}
