package constantin.fpv_vr;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.PowerManager;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import com.google.vr.ndk.base.GvrUiLayout;
import com.google.vr.sdk.base.AndroidCompat;
import com.google.vr.sdk.base.Eye;

public class Activity_Stereo extends AppCompatActivity {
    private GLSurfaceViewEGL14 mGLView14Stereo;
    private GLRenderer14_Stereo mGLRenderer14Stereo;
    private GvrLayout mGvrLayout;
    private GvrApi mGvrApi;

    //We mustn't call onPause/onResume on zhe Gvr layout, or else the surface might be destroyed/app will crash. TODO: track this issue to it's roots
    private final boolean useGVRLayout=true;

    private void enableSustainedPerformanceIfPossible(){
        if (Build.VERSION.SDK_INT >= 24) {
            final PowerManager powerManager = (PowerManager)getSystemService(POWER_SERVICE);
            if(powerManager!=null){
                if (powerManager.isSustainedPerformanceModeSupported()) {
                    //slightly lower, but sustainable clock speeds
                    //I also enable this mode (if the device supports it) when not doing front buffer rendering,
                    //because when the user decides to render at 120fps or more (disable vsync/60fpsCap)
                    //the App benefits from sustained performance, too
                    AndroidCompat.setSustainedPerformanceMode(this,true);
                    System.out.println("Sustained performance set true");
                }else{
                    System.out.println("Sustained performance not available");
                }
            }
        }
    }
    private void disableSustainedPerformanceIfEnabled(){
        if (Build.VERSION.SDK_INT >= 24) {
            final PowerManager powerManager = (PowerManager)getSystemService(POWER_SERVICE);
            if(powerManager!=null){
                if (powerManager.isSustainedPerformanceModeSupported()) {
                    AndroidCompat.setSustainedPerformanceMode(this, false);
                }
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        System.out.println("OnCreate");
        super.onCreate(savedInstanceState);
        int mode = GLSurfaceViewEGL14.MODE_NORMAL;
        if (Settings.DisableVSYNC) {
            mode = GLSurfaceViewEGL14.MODE_VSYNC_OFF;
        } else if (Settings.Disable60fpsCap) {
            mode = GLSurfaceViewEGL14.MODE_UNLIMITED_FPS_BUT_VSYNC_ON;
        }
        Eye eye;
        if (useGVRLayout) {
            mGvrLayout = new GvrLayout(this);
            mGvrLayout.setAsyncReprojectionEnabled(false);
            mGLView14Stereo = new GLSurfaceViewEGL14(this, mode, Settings.MSAALevel);
            mGLRenderer14Stereo = new GLRenderer14_Stereo(this, mGvrLayout.getGvrApi().getNativeGvrContext());
            mGLView14Stereo.setRenderer(mGLRenderer14Stereo);
            mGLView14Stereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
            mGLView14Stereo.setPreserveEGLContextOnPause(false);
            mGvrLayout.setPresentationView(mGLView14Stereo);
            mGvrLayout.getUiLayout().setSettingsButtonEnabled(false);
            setContentView(mGvrLayout);
        } else {
            mGvrApi = new GvrApi(this, new DisplaySynchronizer(this, getWindowManager().getDefaultDisplay()));
            mGvrApi.reconnectSensors();
            mGvrApi.usingVrDisplayService();
            mGvrApi.clearError();
            mGLView14Stereo = new GLSurfaceViewEGL14(this, mode, Settings.MSAALevel);
            mGLRenderer14Stereo = new GLRenderer14_Stereo(this, mGvrApi.getNativeGvrContext());
            mGLView14Stereo.setRenderer(mGLRenderer14Stereo);
            mGLView14Stereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
            mGLView14Stereo.setPreserveEGLContextOnPause(false);
            setContentView(mGLView14Stereo);
        }
    }

    @Override
    protected void onResume() {
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
        enableSustainedPerformanceIfPossible();
        //if(useGVRLayout){
        //    mGvrLayout.onResume();
        //}
        mGLView14Stereo.onResume();
    }

    @Override
    protected void onPause(){
        super.onPause();
        //if(useGVRLayout){
            //calling onResume/onPause on the GvrLayou results in some weird onPause/onSurfaceDestroyed
            //mGvrLayout.onPause();
        //}
        mGLView14Stereo.onPause();
        disableSustainedPerformanceIfEnabled();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        //mGLRenderer14Stereo.onDelete();
        if(useGVRLayout){
            mGvrLayout.shutdown();
        }else{
            mGvrApi.shutdown();
        }
        mGvrLayout=null;
        mGLView14Stereo=null;
        mGLRenderer14Stereo=null;
    }
}