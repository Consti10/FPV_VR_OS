package constantin.fpv_vr.PlayMono;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.MVideoPlayer;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.R;
import constantin.renderingX.MyEGLConfigChooser;
import constantin.renderingX.MyEGLWindowSurfaceFactory;
import constantin.renderingX.PerformanceHelper;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.External.AspectFrameLayout;
import constantin.video.core.IVideoParamsChanged;

/*****************************************************************
 * Play video blended with OSD in a Mono window (e.g. for Tablets usw)
 * The video is displayed with the Android HW composer, not OpenGL
 * See @AMonoGLVideoOSD for rendering video with OpenGL
 * OSD can be fully disabled
 ***************************************************************** */


public class AMonoVideoOSD extends AppCompatActivity implements SurfaceHolder.Callback, IVideoParamsChanged {
    public static final String EXTRA_KEY_ENABLE_OSD="EXTRA_KEY_ENABLE_OSD";
    private AspectFrameLayout mAspectFrameLayout;
    private MVideoPlayer mVideoPlayer;
    private Context mContext;
    private GvrApi gvrApi;
    private AirHeadTrackingSender airHeadTrackingSender;
    //Only !=null when ENABLE_OSD is enabled
    private GLSurfaceView mGLView;
    private GLRMono mGLRMonoOSD;
    private TelemetryReceiver telemetryReceiver;
    private boolean ENABLE_OSD;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ENABLE_OSD =getIntent().getBooleanExtra(EXTRA_KEY_ENABLE_OSD,true);
        mContext=this;
        setContentView(R.layout.activity_mono_vid_osd);
        SurfaceView mSurfaceView = findViewById(R.id.VideoSurfaceBelow);
        mSurfaceView.getHolder().addCallback(this);
        mAspectFrameLayout =  findViewById(R.id.VideoSurface_AFL);
        if(ENABLE_OSD){
            mGLView = new GLSurfaceView(this);
            mGLView.setEGLContextClientVersion(2);
            //Do not use msaa in mono mode
            mGLView.setEGLConfigChooser(new MyEGLConfigChooser(false,0,true));
            mGLView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());
            mGLView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
            mGLView.setPreserveEGLContextOnPause(true);
            telemetryReceiver=new TelemetryReceiver(this);
            mGLRMonoOSD =new GLRMono(mContext,telemetryReceiver,null,GLRMono.VIDEO_MODE_NONE,true,false);
            mGLView.setRenderer(mGLRMonoOSD);
            mGLView.setZOrderMediaOverlay(true);
            addContentView(mGLView,new ActionBar.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT));
        }
        if(SJ.EnableAHT(mContext)){
            gvrApi = new GvrApi(this, new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay()));
            airHeadTrackingSender=new AirHeadTrackingSender(this,gvrApi);
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        //Log.d(TAG, "onResume");
        PerformanceHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        if(ENABLE_OSD){
            telemetryReceiver.startReceiving();
            mGLView.onResume();
        }
        if(gvrApi!=null){
            gvrApi.resumeTracking();
            airHeadTrackingSender.startSendingDataIfEnabled();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        //Log.d(TAG, "onPause");
        if(gvrApi!=null){
            airHeadTrackingSender.stopSendingDataIfEnabled();
            gvrApi.pauseTracking();
        }
        if(ENABLE_OSD){
            telemetryReceiver.stopReceiving();
            mGLView.onPause();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(gvrApi!=null){
            gvrApi.shutdown();
        }
        if(ENABLE_OSD){
            mGLView=null;
            mGLRMonoOSD =null;
            telemetryReceiver.delete();
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // There's a short delay between the start of the activity and the initialization
        // of the SurfaceHolder that backs the SurfaceView. (Video Surface,not OpenGL surface)
        //Log.d(TAG, "Video surface created");
        if(mVideoPlayer==null){
            mVideoPlayer=new MVideoPlayer(mContext,holder.getSurface(),this);
            mVideoPlayer.start();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // ignore
        //Log.d(TAG, "Video surfaceChanged fmt=" + format + " size=" + width + "x" + height);
        //format 4= rgb565
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        //Log.d(TAG, "Video Surface destroyed");
        if(mVideoPlayer!=null){
            mVideoPlayer.stop();
            mVideoPlayer=null;
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


