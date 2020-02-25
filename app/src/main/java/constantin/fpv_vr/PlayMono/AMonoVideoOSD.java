package constantin.fpv_vr.PlayMono;

import android.content.Context;
import android.graphics.PixelFormat;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.view.WindowManager;
import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;
import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.R;
import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyEGLWindowSurfaceFactory;
import constantin.renderingx.core.MyGLSurfaceView;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.External.AspectFrameLayout;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayerSurfaceHolder;

/*****************************************************************
 * Play video blended with OSD in a Mono window (e.g. for Tablets usw)
 * The video is displayed with the Android HW composer, not OpenGL
 * See @AMonoGLVideoOSD for rendering video with OpenGL
 * OSD can be fully disabled
 ***************************************************************** */


public class AMonoVideoOSD extends AppCompatActivity implements IVideoParamsChanged {
    public static final String EXTRA_KEY_ENABLE_OSD="EXTRA_KEY_ENABLE_OSD";
    private AspectFrameLayout mAspectFrameLayout;
    private Context mContext;
    private GvrApi gvrApi;
    private AirHeadTrackingSender airHeadTrackingSender;
    //Only !=null when ENABLE_OSD is enabled
    private MyGLSurfaceView mGLView;
    private TelemetryReceiver telemetryReceiver;
    private boolean ENABLE_OSD;
    private boolean ENABLE_VIDEO_VIA_OPENGL;
    private VideoPlayerSurfaceHolder mVideoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ENABLE_OSD =getIntent().getBooleanExtra(EXTRA_KEY_ENABLE_OSD,true);
        ENABLE_VIDEO_VIA_OPENGL=false;
        mContext=this;
        setContentView(R.layout.activity_mono_vid_osd);
        SurfaceView mSurfaceView = findViewById(R.id.SurfaceView_monoscopicVideo);
        mVideoPlayer=new VideoPlayerSurfaceHolder(mContext,this);
        mSurfaceView.getHolder().addCallback(mVideoPlayer);
        mAspectFrameLayout =  findViewById(R.id.VideoSurface_AFL);
        if(ENABLE_OSD){
            mGLView = new MyGLSurfaceView(this,this);
            mGLView.setEGLContextClientVersion(2);
            //Do not use MSAA in mono mode
            mGLView.setEGLConfigChooser(new MyEGLConfigChooser(false,0,true));
            mGLView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());
            mGLView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
            mGLView.setPreserveEGLContextOnPause(true);
            telemetryReceiver=new TelemetryReceiver(this);
            final GLRMono mGLRMonoOSD = new GLRMono(mContext, null, telemetryReceiver, null, GLRMono.VIDEO_MODE_2D_MONOSCOPIC, true, false);
            mGLView.setRenderer(mGLRMonoOSD);
            mGLView.setZOrderMediaOverlay(true);
            addContentView(mGLView,new ActionBar.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT));
        }
        if(SJ.EnableAHT(mContext)){
            gvrApi = new GvrApi(this, new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay()));
            airHeadTrackingSender=new AirHeadTrackingSender(this,gvrApi);
            Log.d("TAG","AHT LOL");
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        //Log.d(TAG, "onResume");
        FullscreenHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        if(gvrApi!=null){
            gvrApi.resumeTracking();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        //Log.d(TAG, "onPause");
        if(gvrApi!=null){
            gvrApi.pauseTracking();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(gvrApi!=null){
            gvrApi.shutdown();
        }
    }


    @Override
    public void onVideoRatioChanged(final int videoW, final int videoH) {
        //System.out.println("Width:"+videoW+"Height:"+videoH);
        runOnUiThread(new Runnable() {
            public void run() {
                mAspectFrameLayout.setAspectRatio((double) videoW / videoH);
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


