package constantin.fpv_vr.PlayMono;

import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.XDJI.XTelemetryReceiver;
import constantin.fpv_vr.XDJI.XVideoPlayerSurfaceHolder;
import constantin.fpv_vr.XDJI.XVideoPlayerSurfaceTexture;
import constantin.fpv_vr.databinding.ActivityMonoVidOsdBinding;
import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.views.MyEGLConfigChooser;
import constantin.renderingx.core.views.MyEGLWindowSurfaceFactory;
import constantin.renderingx.core.views.MyGLSurfaceView;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer.VideoSettings;

/*****************************************************************
 *  * OSD can be fully disabled
 * Mode 1:
 * Play video blended with OSD in a Mono window (e.g. for Tablets usw)
 * The video is displayed with the Android HW composer, not OpenGL
 * Mode 2:
 * Play video blended with OSD in a Mono window, but without using the HW composer for video (e.g. using SurfaceTexture for it)
 * 360° or stereo video needs to be rendered with OpenGL regardless weather the user wants to see the OSD or not
 ***************************************************************** */

public class AMonoVideoOSD extends AppCompatActivity implements IVideoParamsChanged {
    private ActivityMonoVidOsdBinding binding1;
    private constantin.fpv_vr.databinding.ActivityMonoGlVidOsdBinding bindingGL;
    public static final String EXTRA_KEY_ENABLE_OSD="EXTRA_KEY_ENABLE_OSD";
    private XTelemetryReceiver telemetryReceiver;
    private GLRMono mGLRenderer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final boolean ENABLE_OSD = getIntent().getBooleanExtra(EXTRA_KEY_ENABLE_OSD, true);
        final boolean USE_ANDROID_SURFACE_FOR_VIDEO=VideoSettings.videoMode(this)==0;
        System.out.println("USE_ANDROID_SURFACE_FOR_VIDEO"+USE_ANDROID_SURFACE_FOR_VIDEO);
        if(USE_ANDROID_SURFACE_FOR_VIDEO){
        }else{
        }
        //
        if(USE_ANDROID_SURFACE_FOR_VIDEO){
            binding1 = ActivityMonoVidOsdBinding.inflate(getLayoutInflater());
            setContentView(binding1.getRoot());
            XVideoPlayerSurfaceHolder videoPlayer=new XVideoPlayerSurfaceHolder(this);
            videoPlayer.setIVideoParamsChanged(this);
            binding1.SurfaceViewMonoscopicVideo.getHolder().addCallback(videoPlayer);
            //--

            if(ENABLE_OSD){
                binding1.MyGLSurfaceView.setVisibility(View.VISIBLE);
                binding1.MyGLSurfaceView.setEGLContextClientVersion(2);
                //Do not use MSAA in mono mode
                binding1.MyGLSurfaceView.setEGLConfigChooser(new MyEGLConfigChooser(false,0,true));
                binding1.MyGLSurfaceView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());
                binding1.MyGLSurfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
                binding1.MyGLSurfaceView.setPreserveEGLContextOnPause(true);
                telemetryReceiver=new XTelemetryReceiver(this,videoPlayer.getExternalGroundRecorder());
                mGLRenderer = new GLRMono(this, null, telemetryReceiver, null, GLRMono.VIDEO_MODE_2D_MONOSCOPIC, true, false);
                binding1.MyGLSurfaceView.setRenderer(mGLRenderer);
            }
        }else{
            bindingGL = constantin.fpv_vr.databinding.ActivityMonoGlVidOsdBinding.inflate(getLayoutInflater());
            setContentView(bindingGL.getRoot());
            XVideoPlayerSurfaceTexture videoPlayer=new XVideoPlayerSurfaceTexture(this);
            MyGLSurfaceView mGLView = new MyGLSurfaceView(this);
            mGLView.setEGLContextClientVersion(2);
            //for now do not differentiate
            final boolean disableVSYNC = SJ.DisableVSYNC(this);
            //do not use MSAA in mono mode
            mGLView.setEGLConfigChooser(new MyEGLConfigChooser(disableVSYNC, 0,true));
            mGLView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());

            bindingGL.myVRLayout.setVrOverlayEnabled(false);
            bindingGL.myVRLayout.setPresentationView(mGLView);

            //private TelemetryReceiver telemetryReceiver;
            telemetryReceiver = new XTelemetryReceiver(this, videoPlayer.getExternalGroundRecorder());
            mGLRenderer =new GLRMono(this,videoPlayer, telemetryReceiver, bindingGL.myVRLayout.getGvrApi(),
                    VideoSettings.videoMode(this),ENABLE_OSD, disableVSYNC);
            mGLView.setRenderer(mGLRenderer);

            registerForContextMenu(bindingGL.myVRLayout);
        }
        if(bindingGL ==null){
            AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this);
        }else{
            AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this, bindingGL.myVRLayout.getGvrApi());
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        FullscreenHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
    }


    @Override
    public void onVideoRatioChanged(final int videoW, final int videoH) {
        //System.out.println("Width:"+videoW+"Height:"+videoH);
        runOnUiThread(new Runnable() {
            public void run() {
                binding1.VideoSurfaceAFL.setAspectRatio((double) videoW / videoH);
            }
        });
        if(mGLRenderer!=null){
            mGLRenderer.onVideoRatioChanged(videoW,videoH);
        }
    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        if(telemetryReceiver!=null){
            telemetryReceiver.setDecodingInfo(decodingInfo.currentFPS,decodingInfo.currentKiloBitsPerSecond,decodingInfo.avgParsingTime_ms,decodingInfo.avgWaitForInputBTime_ms,
                    decodingInfo.avgHWDecodingTime_ms);
        }
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
                GvrApi api= bindingGL.myVRLayout.getGvrApi();
                api.recenterTracking();
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }

}


