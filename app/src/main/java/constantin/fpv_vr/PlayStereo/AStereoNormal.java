package constantin.fpv_vr.PlayStereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;

import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.ndk.base.BufferViewport;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyVRLayout;
import constantin.renderingx.core.PerformanceHelper;
import constantin.telemetry.core.TelemetryReceiver;

public class AStereoNormal extends AppCompatActivity {
    //private GvrLayout mVrLayout;
    private MyVRLayout mVrLayout;
    private GLSurfaceView mGLViewStereo;

    private GLRStereoNormal mGLRStereoNormal;
    private AirHeadTrackingSender airHeadTrackingSender;
    private Context mContext;
    private TelemetryReceiver telemetryReceiver;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        //mVrLayout = new GvrLayout(this);
        mVrLayout =new MyVRLayout(this);
        mGLViewStereo=new GLSurfaceView(this);
        mGLViewStereo.setEGLContextClientVersion(2);
        mGLViewStereo.setEGLConfigChooser(new MyEGLConfigChooser(SJ.DisableVSYNC(this),SJ.MultiSampleAntiAliasing(this)));
        //mGLViewStereo.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory(true));
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRStereoNormal = new GLRStereoNormal(this,telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext());

        mGLViewStereo.setRenderer(mGLRStereoNormal);
        mGLViewStereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLViewStereo.setPreserveEGLContextOnPause(true);
        mVrLayout.setPresentationView(mGLViewStereo);
        setContentView(mVrLayout);
        airHeadTrackingSender=new AirHeadTrackingSender(this, mVrLayout.getGvrApi());

        registerForContextMenu(mVrLayout);


        float[] input={0,0};
        float[] output= mVrLayout.getGvrApi().computeDistortedPoint(BufferViewport.EyeType.LEFT,input);
        System.out.println("Distortion:("+output[0]+","+output[1]+","+output[2]+")");

    }

    public static void  bla(Context c){
        GvrView view=new GvrView(c);
        final GvrViewerParams params=view.getGvrViewerParams();
        //params.getDistortion().getApproximateInverseDistortion()
        //System.out.println("Model: "+params.getModel());
    }


    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("YYY onResume()");
        PerformanceHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        PerformanceHelper.enableSustainedPerformanceIfPossible(this);
        telemetryReceiver.startReceiving();
        mVrLayout.onResumeX();
        mGLViewStereo.onResume();
        airHeadTrackingSender.startSendingDataIfEnabled();
    }


    @Override
    protected void onPause(){
        super.onPause();
        System.out.println("YYY onPause()");
        mGLRStereoNormal.onPause();
        telemetryReceiver.stopReceiving();
        airHeadTrackingSender.stopSendingDataIfEnabled();
        mVrLayout.onPauseX();
        mGLViewStereo.onPause();
        PerformanceHelper.disableSustainedPerformanceIfEnabled(this);
    }


    @Override
    protected void onDestroy(){
        super.onDestroy();
        mVrLayout.shutdown();
        mVrLayout =null;
        mGLViewStereo =null;
        mGLRStereoNormal=null;
        airHeadTrackingSender=null;
        telemetryReceiver.delete();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        menu.setHeaderTitle("Options");
        getMenuInflater().inflate(R.menu.videovr_context_menu, menu);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.option_reset_tracking:
               mVrLayout.getGvrApi().recenterTracking();
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }


}