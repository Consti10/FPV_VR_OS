package constantin.fpv_vr.PlayStereo;
import android.app.Activity;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrUiLayout;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.PlayStereo.GLRStereoSuperSync;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.Toaster;
import constantin.renderingx.core.PerformanceHelper;
import constantin.renderingx.core.ViewSuperSync;
import constantin.telemetry.core.TelemetryReceiver;

/*****************************************
 * Render Video & OSD Side by Side. Difference to AStereoNormal: Renders directly into the Front Buffer  (FB) for lower latency
 * Synchronisation with the VSYNC is done in cpp.
 * Pipeline h.264-->image on screen:
 * h.264 NALUs->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 **************************************** */

public class AStereoSuperSYNC extends AppCompatActivity{
    private ViewSuperSync mViewSuperSync;
    private GLRStereoSuperSync mGLRStereoSuperSync;
    private AirHeadTrackingSender airHeadTrackingSender;
    private TelemetryReceiver telemetryReceiver;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mViewSuperSync=new ViewSuperSync(this);
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRStereoSuperSync = new GLRStereoSuperSync(this,telemetryReceiver,mViewSuperSync.getGvrApi().getNativeGvrContext());
        mViewSuperSync.setRenderer(mGLRStereoSuperSync);

        setContentView(mViewSuperSync);
        airHeadTrackingSender=new AirHeadTrackingSender(this,mViewSuperSync.getGvrApi());
    }

    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("YYY onResume()");
        PerformanceHelper.setImmersiveSticky(this);
        if(!SJ.ENABLE_LOW_PERSISTENCE(this)){
            //PerformanceHelper.enableAndroidVRModeIfPossible(this);
        }
        PerformanceHelper.enableSustainedPerformanceIfPossible(this);
        telemetryReceiver.startReceiving();
        mViewSuperSync.onResume();
        airHeadTrackingSender.startSendingDataIfEnabled();
    }

    @Override
    protected void onPause(){
        super.onPause();
        System.out.println("YYY onPause()");
        telemetryReceiver.stopReceiving();
        airHeadTrackingSender.stopSendingDataIfEnabled();
        mViewSuperSync.onPause();
        PerformanceHelper.disableSustainedPerformanceIfEnabled(this);
        //PerformanceHelper.disableAndroidVRModeIfEnabled(this);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mViewSuperSync.destroy();
        mViewSuperSync=null;
        mGLRStereoSuperSync=null;
        airHeadTrackingSender=null;
        telemetryReceiver.delete();
    }

}


