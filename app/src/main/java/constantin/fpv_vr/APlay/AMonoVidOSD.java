package constantin.fpv_vr.APlay;

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
import constantin.fpv_vr.GLRenderer.GLRMono;
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
 ***************************************************************** */


public class AMonoVidOSD extends AppCompatActivity implements SurfaceHolder.Callback, IVideoParamsChanged {
    private AspectFrameLayout mAspectFrameLayout;
    private MVideoPlayer mVideoPlayer;
    private Context mContext;
    private GLSurfaceView mGLView;
    private GvrApi gvrApi;
    private GLRMono mGLRMono;
    private AirHeadTrackingSender airHeadTrackingSender;
    private TelemetryReceiver telemetryReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        setContentView(R.layout.activity_mono_vid_osd);
        SurfaceView mSurfaceView = findViewById(R.id.VideoSurfaceBelow);
        mSurfaceView.getHolder().addCallback(this);
        mAspectFrameLayout =  findViewById(R.id.VideoSurface_AFL);
        mGLView = new GLSurfaceView(this);
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(new MyEGLConfigChooser(false,SJ.MultiSampleAntiAliasing(this),true));
        mGLView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());

        mGLView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        mGLView.setPreserveEGLContextOnPause(true);
        if(SJ.EnableAHT(mContext)){
            gvrApi = new GvrApi(this, new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay()));
            airHeadTrackingSender=new AirHeadTrackingSender(this,gvrApi);
        }
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRMono=new GLRMono(mContext,telemetryReceiver);
        mGLView.setRenderer(mGLRMono);
        mGLView.setZOrderMediaOverlay(true);
        addContentView(mGLView,new ActionBar.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
    }


    @Override
    protected void onResume() {
        super.onResume();
        //Log.d(TAG, "onResume");
        PerformanceHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        telemetryReceiver.startReceiving();
        mGLView.onResume();
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
        telemetryReceiver.stopReceiving();
        mGLView.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(gvrApi!=null){
            gvrApi.shutdown();
        }
        mGLView=null;
        mGLRMono=null;
        telemetryReceiver.delete();
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
        telemetryReceiver.setDecodingInfo(decodingInfo.currentFPS,decodingInfo.currentKiloBitsPerSecond,decodingInfo.avgParsingTime_ms,decodingInfo.avgWaitForInputBTime_ms,
                decodingInfo.avgHWDecodingTime_ms);
    }

}


