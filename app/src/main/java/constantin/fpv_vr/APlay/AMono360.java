package constantin.fpv_vr.APlay;

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

import constantin.fpv_vr.GLRenderer.GLRMono360;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingX.MyEGLConfigChooser;
import constantin.renderingX.MyEGLWindowSurfaceFactory;
import constantin.renderingX.PerformanceHelper;
import constantin.telemetry.core.TelemetryReceiver;

//The 360° video needs to be transformed by OpenGL regardless weather the user wants to see the OSD or not
public class AMono360 extends AppCompatActivity {
    private Context mContext;
    private GLSurfaceView mGLView;
    private GLRMono360 mGLRenderer;
    private TelemetryReceiver telemetryReceiver;
    //either create gvr api directly or use gvr layout as wrapper
    //First one caused bugs when not forwarding onResume/Pause to the DisplaySynchronizer, see https://github.com/googlevr/gvr-android-sdk/issues/556
    private static final boolean useGvrLayout=true;
    private GvrApi gvrApi;
    private GvrLayout gvrLayout;
    public static final String EXTRA_RENDER_OSD ="EXTRA_RENDER_OSD"; //boolean weather OSD should be enabled
    private DisplaySynchronizer displaySynchronizer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        final boolean renderOSD=getIntent().getBooleanExtra(EXTRA_RENDER_OSD,true);
        PerformanceHelper.enableSustainedPerformanceIfPossible(this);
        mGLView = new GLSurfaceView(this);
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(new MyEGLConfigChooser(false, SJ.MultiSampleAntiAliasing(this),true));
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
        mGLRenderer =new GLRMono360(mContext,telemetryReceiver,useGvrLayout ? gvrLayout.getGvrApi() : gvrApi,renderOSD);
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
        getMenuInflater().inflate(R.menu.video_context_menu, menu);
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
