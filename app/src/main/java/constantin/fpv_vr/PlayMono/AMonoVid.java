package constantin.fpv_vr.PlayMono;

import android.content.Context;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.MVideoPlayer;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingX.PerformanceHelper;
import constantin.video.core.DecodingInfo;
import constantin.video.core.External.AspectFrameLayout;
import constantin.video.core.IVideoParamsChanged;

/*****************************************************************
 * Play video only in a Mono window (e.g. for Tablets,checking Video Quality).
 * 'Normal' video is displayed with the Android HW composer
 * 'Stereo' (e.g. one video frame for left eye,and one video frame for right eye) and
 * '360 Degree video' is rendered with OpenGL.
 ******************************************************************/


public class AMonoVid extends AppCompatActivity implements SurfaceHolder.Callback, IVideoParamsChanged {
    private AspectFrameLayout mAspectFrameLayout;
    private MVideoPlayer mVideoPlayer;
    private Context mContext;
    private GvrApi gvrApi;
    private AirHeadTrackingSender airHeadTrackingSender;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mono_video);
        mContext=this;
        SurfaceView mSurfaceView = findViewById(R.id.videoOnly_Surface);
        //mSurfaceView.getHolder().setFixedSize(120*2,120);
        mSurfaceView.getHolder().addCallback(this);
        mAspectFrameLayout =  findViewById(R.id.videoOnly_AFL);

        if(SJ.EnableAHT(mContext)){
            gvrApi=new GvrApi(this, new DisplaySynchronizer(this, getWindowManager().getDefaultDisplay()));
            airHeadTrackingSender=new AirHeadTrackingSender(this,gvrApi);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        PerformanceHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
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
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        if(gvrApi!=null){
            gvrApi.shutdown();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // There's a short delay between the start of the activity and the initialization
        // of the SurfaceHolder that backs the SurfaceView.
        //Log.d(TAG, "surfaceCreated");
        if(mVideoPlayer==null){
            mVideoPlayer=new MVideoPlayer(mContext,holder.getSurface(),this);
            mVideoPlayer.start();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d("AMonoVid", "surfaceChanged fmt=" + format + " size=" + width + "x" + height);
        //format 4= rgb565
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        //Log.d(TAG, "Surface destroyed");
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

    }

}


