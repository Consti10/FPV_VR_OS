package constantin.fpv_vr;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.PowerManager;
import android.os.Process;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Choreographer;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;
import com.google.vr.sdk.base.AndroidCompat;
import com.google.vr.sdk.base.GvrView;

/*****************************************
 * Render Video & OSD Side by Side. Difference to Activity_Stereo: Renders directly into the Front Buffer  (FB) for lower latency
 * Synchronisation with the VSYNC is done in cpp.
 * Pipeline h.264-->image on screen:
 * h.264 NALUs->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 *****************************************/

public class Activity_StereoSuperSYNC extends AppCompatActivity implements Choreographer.FrameCallback{
    private GLSurfaceViewEGL14 mGLView14_FB; //14 stands for EGL14
    private GLRenderer14_StereoSuperSYNC mGLRenderer14Stereo_SuperSYNC;
    private GvrLayout mGvrLayout;
    private GvrApi mGvrApi;
    private HomeLocation mHomeLocation;

    //We mustn't call onPause/onResume on the Gvr layout, or else the surface might be destroyed/app will crash. TODO: track this issue to it's roots
    private final boolean useGVRLayout=true;

    private void enableVRStuffIfPossibe(){
        if (Build.VERSION.SDK_INT >= 24) {
            final PowerManager powerManager = (PowerManager)getSystemService(POWER_SERVICE);
            if(powerManager!=null){
                if (powerManager.isSustainedPerformanceModeSupported()) {
                    //slightly lower, but sustainable clock speeds
                    AndroidCompat.setSustainedPerformanceMode(this,true);
                    System.out.println("Sustained performance set true");
                    //get the exclusive VR Core
                    /*int[] cores= Process.getExclusiveCores();
                    if(cores!=null){
                        if(cores.length>=1){
                            System.out.println("N exclusive Cores:"+cores.length+" reserved core number:"+cores[0]);
                        }
                    }*/
                }else{
                    System.out.println("Sustained performance not available");
                }
            }
            //I think this sets the display into low-persistence mode for example
            if(!AndroidCompat.setVrModeEnabled(this,true)){
                System.out.println("Cannot enable vr mode");
            }else{
                System.out.println("vr mode enabled");
            }
        }
    }

    private void disableVRStuffIfEnabled(){
        if (Build.VERSION.SDK_INT >= 24) {
            final PowerManager powerManager = (PowerManager)getSystemService(POWER_SERVICE);
            if(powerManager!=null){
                if (powerManager.isSustainedPerformanceModeSupported()) {
                    AndroidCompat.setSustainedPerformanceMode(this, false);
                }
            }
            AndroidCompat.setVrModeEnabled(this,false);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        enableVRStuffIfPossibe();
        if(useGVRLayout){
            mGLView14_FB = new GLSurfaceViewEGL14(this,GLSurfaceViewEGL14.MODE_SYNCHRONOUS_FRONT_BUFFER_RENDERING,Settings.MSAALevel);
            mGvrLayout=new GvrLayout(this);
            mGvrLayout.setAsyncReprojectionEnabled(false);
            mGLRenderer14Stereo_SuperSYNC = new GLRenderer14_StereoSuperSYNC(this,mGvrLayout.getGvrApi().getNativeGvrContext());
            mGLView14_FB.setRenderer(mGLRenderer14Stereo_SuperSYNC);
            mGLView14_FB.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
            mGLView14_FB.setPreserveEGLContextOnPause(false);
            mGvrLayout.setPresentationView(mGLView14_FB);
            mGvrLayout.getUiLayout().setSettingsButtonEnabled(false);
            setContentView(mGvrLayout);
        }else{
            mGvrApi = new GvrApi(this, new DisplaySynchronizer(this, getWindowManager().getDefaultDisplay()));
            mGvrApi.reconnectSensors();
            mGvrApi.usingVrDisplayService();
            mGvrApi.clearError();
            mGLView14_FB = new GLSurfaceViewEGL14(this,GLSurfaceViewEGL14.MODE_SYNCHRONOUS_FRONT_BUFFER_RENDERING,Settings.MSAALevel);
            mGLRenderer14Stereo_SuperSYNC = new GLRenderer14_StereoSuperSYNC(this,mGvrApi.getNativeGvrContext());
            mGLView14_FB.setRenderer(mGLRenderer14Stereo_SuperSYNC);
            mGLView14_FB.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
            mGLView14_FB.setPreserveEGLContextOnPause(false);
            setContentView(mGLView14_FB);
        }
        mHomeLocation=new HomeLocation(this,mGLRenderer14Stereo_SuperSYNC);
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
        //Enable sustained performance and vr mode if possible
        enableVRStuffIfPossibe();
        mGLView14_FB.onResume();
        mGLView14_FB.requestRender();
        Choreographer.getInstance().postFrameCallback(this);
        //System.out.println("OGLActivity" + "On Resume");
        //if(useGVRLayout){
        //mGvrLayout.onResume();
        //}
        mHomeLocation.resume();
    }

    @Override
    protected void onPause(){
        super.onPause();
        GLRenderer14_StereoSuperSYNC.nativeExitSuperSyncLoop(mGLRenderer14Stereo_SuperSYNC.nativeGLRSuperSync);
        Choreographer.getInstance().removeFrameCallback(this);
        mGLView14_FB.onPause();
        disableVRStuffIfEnabled();
        //if(useGVRLayout){
        //    mGvrLayout.onPause();
        //}
        //System.out.println("OGLActivity" + "On Pause");
        mHomeLocation.pause();
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        if(useGVRLayout){
            if(mGvrLayout!=null){
                mGvrLayout.shutdown();
            }
        }else{
            if(mGvrApi!=null){
                mGvrApi.shutdown();
            }
        }
        mGvrLayout=null;
        mGLView14_FB=null;
        mGLRenderer14Stereo_SuperSYNC=null;
    }


    @Override
    public void doFrame(long frameTimeNanos) {
        mGLRenderer14Stereo_SuperSYNC.doFramePasstrough(frameTimeNanos);
        Choreographer.getInstance().postFrameCallback(this);
    }
}


