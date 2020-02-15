package constantin.fpv_vr.PlayMono;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.renderingx.core.ISurfaceTextureAvailable;
import constantin.fpv_vr.MVideoPlayer;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyEGLWindowSurfaceFactory;
import constantin.renderingx.core.MyVRLayout;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.VideoNative.VideoNative;


/*****************************************************************
 * Play video blended with OSD in a Mono window, but without using the HW composer for video
 * 360° or stereo video needs to be rendered with OpenGL regardless weather the user wants to see the OSD or not
 * OSD can be fully disabled
 ***************************************************************** */

public class AMonoGLVideoOSD extends AppCompatActivity implements ISurfaceTextureAvailable {
    private static final String TAG="AMonoGLVideoOSD";
    private Context mContext;
    private GLSurfaceView mGLView;
    private GLRMono mGLRenderer;
    private TelemetryReceiver telemetryReceiver;
    private static final boolean useGvrLayout=false;
    private GvrLayout gvrLayout;
    private MyVRLayout myVRLayout;
    public static final String EXTRA_RENDER_OSD ="EXTRA_RENDER_OSD"; //boolean weather ENABLE_OSD should be enabled
    private MVideoPlayer mVideoPlayer;
    private SurfaceTexture surfaceTexture;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        final boolean renderOSD=getIntent().getBooleanExtra(EXTRA_RENDER_OSD,true);
        MyVRLayout.enableSustainedPerformanceIfPossible(this);
        mGLView = new GLSurfaceView(this);
        mGLView.setEGLContextClientVersion(2);
        //for now do not differentiate
        final boolean disableVSYNC = SJ.DisableVSYNC(this);
        //do not use MSAA in mono mode
        mGLView.setEGLConfigChooser(new MyEGLConfigChooser(disableVSYNC, 0,true));
        mGLView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());
        mGLView.setPreserveEGLContextOnPause(true);
        if(useGvrLayout){
            gvrLayout=new GvrLayout(this);
            gvrLayout.setAsyncReprojectionEnabled(false);
            gvrLayout.setStereoModeEnabled(false);
            gvrLayout.setPresentationView(mGLView);
        }else{
            myVRLayout=new MyVRLayout(this);
            myVRLayout.setVrOverlayEnabled(false);
            myVRLayout.setPresentationView(mGLView);
        }
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRenderer =new GLRMono(mContext,this,telemetryReceiver,useGvrLayout ? gvrLayout.getGvrApi() : myVRLayout.getGvrApi(),
                VideoNative.videoMode(mContext),renderOSD, disableVSYNC);
        mGLView.setRenderer(mGLRenderer);
        if(useGvrLayout){
            setContentView(gvrLayout);
            registerForContextMenu(gvrLayout);
        }else{
            setContentView(myVRLayout);
            registerForContextMenu(myVRLayout);
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        //Log.d(TAG, "onResume");
        FullscreenHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        telemetryReceiver.startReceiving();
        mGLView.onResume();
        if(useGvrLayout){
            gvrLayout.onResume();
        }else{
            myVRLayout.onResumeX();
        }
        startVideoIfNotYetStarted();
    }

    @Override
    protected void onPause() {
        super.onPause();
        telemetryReceiver.stopReceiving();
        mGLView.onPause();
        if(useGvrLayout){
            gvrLayout.onPause();
        }else{
            myVRLayout.onPauseX();
        }
        stopVideoIfNotYetSopped();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        if(useGvrLayout){
            gvrLayout.shutdown();
        }else{
            myVRLayout.shutdown();
        }
        telemetryReceiver.delete();
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
                GvrApi api=useGvrLayout ? gvrLayout.getGvrApi() : myVRLayout.getGvrApi();
                api.recenterTracking();
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }

    @Override
    public void onSurfaceTextureAvailable(final SurfaceTexture surfaceTexture) {
        //Start and stop the video on the UI thread only
        final AMonoGLVideoOSD instance=this;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                instance.surfaceTexture=surfaceTexture;
                startVideoIfNotYetStarted();
            }
        });
    }

    //Needs to be called on the UI thread !
    private void startVideoIfNotYetStarted(){
        if(surfaceTexture!=null && mVideoPlayer==null){
            final Surface mVideoSurface=new Surface(surfaceTexture);
            mVideoPlayer=new MVideoPlayer(mContext,mVideoSurface,mGLRenderer);
            mVideoPlayer.start();
        }
    }

    //Needs to be called on the UI thread !
    private void stopVideoIfNotYetSopped(){
        if(mVideoPlayer!=null){
            mVideoPlayer.stop();
            mVideoPlayer=null;
        }
    }
}
