package constantin.fpv_vr.APlay;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.WindowManager;

import com.google.vr.cardboard.DisplaySynchronizer;
import com.google.vr.ndk.base.GvrApi;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.GLRenderer.GLRMono360;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingX.MyEGLConfigChooser;
import constantin.renderingX.MyEGLWindowSurfaceFactory;
import constantin.renderingX.PerformanceHelper;
import constantin.telemetry.core.TelemetryReceiver;

public class AMono360 extends AppCompatActivity {
    private Context mContext;
    private GLSurfaceView mGLView;
    private GvrApi gvrApi;
    private GLRMono360 mGLRenderer;
    private AirHeadTrackingSender airHeadTrackingSender;
    private TelemetryReceiver telemetryReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        mGLView = new GLSurfaceView(this);
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(new MyEGLConfigChooser(false, SJ.MultiSampleAntiAliasing(this),true));
        mGLView.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory());

        //mGLView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        mGLView.setPreserveEGLContextOnPause(true);
        gvrApi = new GvrApi(this, new DisplaySynchronizer(this,getWindowManager().getDefaultDisplay()));
        if(SJ.EnableAHT(mContext)){
            airHeadTrackingSender=new AirHeadTrackingSender(this,gvrApi);
        }
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRenderer =new GLRMono360(mContext,telemetryReceiver,gvrApi);
        mGLView.setRenderer(mGLRenderer);
        setContentView(mGLView);
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
        }
        if(airHeadTrackingSender!=null){
            airHeadTrackingSender.startSendingDataIfEnabled();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        //Log.d(TAG, "onPause");
        if(gvrApi!=null){
            gvrApi.pauseTracking();
        }
        if(airHeadTrackingSender!=null){
            airHeadTrackingSender.stopSendingDataIfEnabled();
        }
        telemetryReceiver.stopReceiving();
        mGLRenderer.onPause();
        mGLView.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(gvrApi!=null){
            gvrApi.shutdown();
        }
        mGLView=null;
        mGLRenderer =null;
        telemetryReceiver.delete();
    }

}
