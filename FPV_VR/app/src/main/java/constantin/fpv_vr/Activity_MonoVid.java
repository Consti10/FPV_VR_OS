package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;

/*****************************************************************
 * Play video only in a Mono window (e.g. for Tablets,checking Video Quality).
 ******************************************************************/


public class Activity_MonoVid extends AppCompatActivity implements SurfaceHolder.Callback,VideoPlayerInterface {
    private static final String TAG = "VideoOnlyActivity";
    private AspectFrameLayout mAspectFrameLayout;
    private SurfaceView mSurfaceView;
    private VideoPlayer mVideoPlayer;
    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mono_video);
        mContext=this;
        mSurfaceView = findViewById(R.id.videoOnly_Surface);
        mSurfaceView.getHolder().addCallback(this);
        mAspectFrameLayout =  findViewById(R.id.videoOnly_AFL);
        mAspectFrameLayout.setAspectRatio(1280.0 / 720.0);
    }

    @Override
    protected void onResume() {
        //Log.d(TAG, "onResume");
        super.onResume();
        //Hide the Action Bar & go into immersive fullscreen sticky mode
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.hide();
        }
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    protected void onPause() {
        super.onPause();
        //Log.d(TAG, "onPause");
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // There's a short delay between the start of the activity and the initialization
        // of the SurfaceHolder that backs the SurfaceView.
        //Log.d(TAG, "surfaceCreated");
        if(mVideoPlayer==null){
            mVideoPlayer=VideoPlayer.createAndStartVideoPlayer(mContext,holder.getSurface(),this);
        }
    }
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        //Log.d(TAG, "surfaceChanged fmt=" + format + " size=" + width + "x" + height);
        //format 4= rgb565
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "Surface destroyed");
        if(mVideoPlayer!=null){
            VideoPlayer.stopAndDeleteVideoPlayer(mVideoPlayer);
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
    public void onVideoFPSChanged(float decFPS) {
        //nothing
    }
}


