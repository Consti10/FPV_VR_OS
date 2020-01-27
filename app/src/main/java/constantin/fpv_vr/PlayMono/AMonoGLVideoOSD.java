package constantin.fpv_vr.PlayMono;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyEGLWindowSurfaceFactory;
import constantin.renderingx.core.PerformanceHelper;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.VideoNative.VideoNative;


/*****************************************************************
 * Play video blended with OSD in a Mono window, but without using the HW composer for video
 * 360° or stereo video needs to be rendered with OpenGL regardless weather the user wants to see the OSD or not
 * OSD can be fully disabled
 ***************************************************************** */

public class AMonoGLVideoOSD extends AppCompatActivity {
    private Context mContext;
    private GLSurfaceView mGLView;
    private GLRMono mGLRenderer;
    private TelemetryReceiver telemetryReceiver;
    //either create gvr api directly or use gvr layout as wrapper
    //First one caused bugs when not forwarding onResume/Pause to the DisplaySynchronizer, see https://github.com/googlevr/gvr-android-sdk/issues/556
    private static final boolean useGvrLayout=true;
    private GvrApi gvrApi;
    private GvrLayout gvrLayout;
    public static final String EXTRA_RENDER_OSD ="EXTRA_RENDER_OSD"; //boolean weather ENABLE_OSD should be enabled
    private DisplaySynchronizer displaySynchronizer;
    private boolean disableVSYNC;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        final boolean renderOSD=getIntent().getBooleanExtra(EXTRA_RENDER_OSD,true);
        PerformanceHelper.enableSustainedPerformanceIfPossible(this);
        mGLView = new GLSurfaceView(this);
        mGLView.setEGLContextClientVersion(2);
        //for now do not differentiate
        disableVSYNC=SJ.DisableVSYNC(this);
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
            //gvrApi = new GvrApi(this, new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay()));
            displaySynchronizer=new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay());
            gvrApi = new GvrApi(this, displaySynchronizer);
            gvrApi.reconnectSensors();
            gvrApi.clearError();
            gvrApi.recenterTracking();
        }
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRenderer =new GLRMono(mContext,telemetryReceiver,useGvrLayout ? gvrLayout.getGvrApi() : gvrApi,
                VideoNative.videoMode(mContext)==1 ?
                        GLRMono.VIDEO_MODE_STEREO:
                        GLRMono.VIDEO_MODE_360 ,renderOSD,disableVSYNC);
        mGLView.setRenderer(mGLRenderer);
        if(useGvrLayout){
            setContentView(gvrLayout);
            registerForContextMenu(gvrLayout);
        }else{
            setContentView(mGLView);
            registerForContextMenu(mGLView);
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        //Log.d(TAG, "onResume");
        PerformanceHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        telemetryReceiver.startReceiving();
        mGLView.onResume();
        if(useGvrLayout){
            gvrLayout.onResume();
        }else{
            gvrApi.resumeTracking();
            displaySynchronizer.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        telemetryReceiver.stopReceiving();
        mGLRenderer.onPause();
        mGLView.onPause();
        if(useGvrLayout){
            gvrLayout.onPause();
        }else{
            gvrApi.pauseTracking();
            displaySynchronizer.onPause();

        }
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
                GvrApi api=useGvrLayout ? gvrLayout.getGvrApi() : gvrApi;
                api.recenterTracking();
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(useGvrLayout){
            gvrLayout.shutdown();
        }else{
            gvrApi.shutdown();
            displaySynchronizer.shutdown();
        }
        mGLView=null;
        mGLRenderer =null;
        telemetryReceiver.delete();
    }

}
