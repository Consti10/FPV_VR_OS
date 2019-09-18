package constantin.fpv_vr.APlay;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;
import com.google.vr.ndk.base.GvrLayout;

import constantin.fpv_vr.GLRenderer.GLRMono360;
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
    //either create gvr api directly or use gvr layout as wrapper - first one introduces bug, e.g. "wrong 360° video orientation"
    private static final boolean useGvrLayout=true;
    private GvrApi gvrApi;
    private GvrLayout gvrLayout;
    public static final String EXTRA_RENDER_OSD ="EXTRA_RENDER_OSD"; //boolean weather OSD should be enabled

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
            gvrApi = new GvrApi(this, new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay()));
            gvrApi.reconnectSensors();
            gvrApi.clearError();
            gvrApi.recenterTracking();
        }
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRenderer =new GLRMono360(mContext,telemetryReceiver,useGvrLayout ? gvrLayout.getGvrApi() : gvrApi,renderOSD);
        mGLView.setRenderer(mGLRenderer);
        if(useGvrLayout){
            setContentView(gvrLayout);
        }else{
            setContentView(mGLView);
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
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(useGvrLayout){
            gvrLayout.shutdown();
        }else{
            gvrApi.shutdown();
        }
        mGLView=null;
        mGLRenderer =null;
        telemetryReceiver.delete();
    }

}
