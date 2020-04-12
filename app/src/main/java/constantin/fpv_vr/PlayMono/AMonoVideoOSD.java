package constantin.fpv_vr.PlayMono;

import android.graphics.PixelFormat;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.XDJI.DJIVideoPlayerSurfaceHolder;
import constantin.fpv_vr.databinding.ActivityMonoVidOsdBinding;
import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyEGLWindowSurfaceFactory;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayerSurfaceHolder;
/*****************************************************************
 * Play video blended with OSD in a Mono window (e.g. for Tablets usw)
 * The video is displayed with the Android HW composer, not OpenGL
 * See @AMonoGLVideoOSD for rendering video with OpenGL
 * OSD can be fully disabled
 ***************************************************************** */


public class AMonoVideoOSD extends AppCompatActivity implements IVideoParamsChanged{
    private ActivityMonoVidOsdBinding binding;
    public static final String EXTRA_KEY_ENABLE_OSD="EXTRA_KEY_ENABLE_OSD";
    private AirHeadTrackingSender airHeadTrackingSender;
    private TelemetryReceiver telemetryReceiver;
    private boolean ENABLE_OSD;
    private boolean ENABLE_VIDEO_VIA_OPENGL;
    //private VideoPlayerSurfaceHolder mVideoPlayer;
    private DJIVideoPlayerSurfaceHolder mVideoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ENABLE_OSD =getIntent().getBooleanExtra(EXTRA_KEY_ENABLE_OSD,true);
        ENABLE_VIDEO_VIA_OPENGL=false;
        binding = ActivityMonoVidOsdBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        //mVideoPlayer=new VideoPlayerSurfaceHolder(this,binding.SurfaceViewMonoscopicVideo,this);
        mVideoPlayer=new DJIVideoPlayerSurfaceHolder(this,binding.SurfaceViewMonoscopicVideo);
        mVideoPlayer.setIVideoParamsChanged(this);
        if(ENABLE_OSD){
            binding.MyGLSurfaceView.setVisibility(View.VISIBLE);
            binding.MyGLSurfaceView.setEGLContextClientVersion(2);
            //Do not use MSAA in mono mode
            binding.MyGLSurfaceView.setEGLConfigChooser(new MyEGLConfigChooser(false,0,true));
            binding.MyGLSurfaceView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());
            binding.MyGLSurfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
            binding.MyGLSurfaceView.setPreserveEGLContextOnPause(true);
            telemetryReceiver=new TelemetryReceiver(this,mVideoPlayer.GetExternalGroundRecorder());
            final GLRMono mGLRMonoOSD = new GLRMono(this, null, telemetryReceiver, null, GLRMono.VIDEO_MODE_2D_MONOSCOPIC, true, false);
            binding.MyGLSurfaceView.setRenderer(mGLRMonoOSD);
        }
        airHeadTrackingSender=AirHeadTrackingSender.createIfEnabled(this);
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
                binding.VideoSurfaceAFL.setAspectRatio((double) videoW / videoH);
            }
        });
    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        if(ENABLE_OSD){
            telemetryReceiver.setDecodingInfo(decodingInfo.currentFPS,decodingInfo.currentKiloBitsPerSecond,decodingInfo.avgParsingTime_ms,decodingInfo.avgWaitForInputBTime_ms,
                    decodingInfo.avgHWDecodingTime_ms);
        }
    }

}


