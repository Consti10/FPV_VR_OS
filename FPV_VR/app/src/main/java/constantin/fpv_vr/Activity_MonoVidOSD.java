package constantin.fpv_vr;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.PixelFormat;
import android.location.Location;
import android.opengl.GLSurfaceView;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/*****************************************************************
 * Play video blended with OSD in a Mono window (e.g. for Tablets usw)
 ***************************************************************** */


public class Activity_MonoVidOSD extends AppCompatActivity implements SurfaceHolder.Callback,VideoPlayer.VideoParamsChanged,HomeLocation.HomeLocationChanged {
    private static final String TAG = "MonoVidOSDActivity";
    static {
        System.loadLibrary("GLRendererMono");
    }
    private static native long nativeConstructRenderer(ClassLoader appClassLoader, Context context, long nativeGvrContext);
    private static native long nativeDestroyRenderer(long glRendererMonoP);
    //surfaceCreated->surfaceChanged->drawFrame->pause->repeat
    private static native void nativeOnSurfaceCreated(long glRendererMonoP,AssetManager assetManager);
    private static native void nativeOnSurfaceChanged(long glRendererMonoP,int width,int height);
    private static native void nativeOnDrawFrame(long glRendererMonoP);
    private static native void nativeSetVideoDecoderFPS(long glRendererMonoP,float fps);
    //since we do not preserve the OpenGL context when paused, nativeOnSurfaceCreated is called each time nativeOnPause was called
    private static native void nativeOnPause(long glRendererMonoP);
    private static native void nativeSetHomeLocation(long glRendererMonoP,double latitude, double longitude,double attitude);
    // Opaque native pointer to the native GLRendererMono instance.
    private long nativeGLRendererMono=0;

    private AspectFrameLayout mAspectFrameLayout;
    private SurfaceView mSurfaceView;
    private VideoPlayer mVideoPlayer;
    private Context mContext;
    private  GLSurfaceView mGLView;

    private HomeLocation mHomeLocation;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mono_vid_osd);
        mContext=this;
        mSurfaceView = findViewById(R.id.VideoSurfaceBelow);
        mSurfaceView.getHolder().addCallback(this);
        mAspectFrameLayout =  findViewById(R.id.VideoSurface_AFL);
        mAspectFrameLayout.setAspectRatio((double)1280/720);

        mGLView = new GLSurfaceView(this);
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        mGLView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        mGLView.setPreserveEGLContextOnPause(false);
        GvrApi gvrApi = new GvrApi(this, new DisplaySynchronizer(this, getWindowManager().getDefaultDisplay()));
        nativeGLRendererMono=nativeConstructRenderer(getClass().getClassLoader(),
                this.getApplicationContext(),
                gvrApi.getNativeGvrContext());

        mGLView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                AssetManager assetManager=mContext.getAssets();
                nativeOnSurfaceCreated(nativeGLRendererMono,assetManager);
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                nativeOnSurfaceChanged(nativeGLRendererMono,width,height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                nativeOnDrawFrame(nativeGLRendererMono);
            }
        });

        mGLView.setZOrderMediaOverlay(true);
        addContentView(mGLView,new ActionBar.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
        mHomeLocation=new HomeLocation(this,this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        //Log.d(TAG, "onResume");
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
        mGLView.onResume();
        mHomeLocation.resume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        //Log.d(TAG, "onPause");
        mHomeLocation.pause();
        mGLView.onPause();
        nativeOnPause(nativeGLRendererMono);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        nativeDestroyRenderer(nativeGLRendererMono);
        nativeGLRendererMono=0;
        mGLView=null;
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // There's a short delay between the start of the activity and the initialization
        // of the SurfaceHolder that backs the SurfaceView. (Video Surface,not OpenGL surface)
        //Log.d(TAG, "Video surface created");
        if(mVideoPlayer==null){
            mVideoPlayer=VideoPlayer.createAndStartVideoPlayer(mContext,holder.getSurface(),this);
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
            VideoPlayer.stopAndDeleteVideoPlayer(mVideoPlayer);
            mVideoPlayer=null;
        }
    }


    @Override
    public void onVideoRatioChanged(final int videoW, final int videoH) {
        System.out.println("Width:"+videoW+"Height:"+videoH);
        runOnUiThread(new Runnable() {
            public void run() {
                mAspectFrameLayout.setAspectRatio((double) videoW / videoH);
            }
        });
    }

    @Override
    public void onVideoFPSChanged(float decFPS) {
        if(nativeGLRendererMono!=0){
            nativeSetVideoDecoderFPS(nativeGLRendererMono,decFPS);
        }
    }


    @Override
    public void onHomeLocationChanged(Location location) {
        nativeSetHomeLocation(nativeGLRendererMono,location.getLatitude(),location.getLongitude(),location.getAltitude());
    }
}


