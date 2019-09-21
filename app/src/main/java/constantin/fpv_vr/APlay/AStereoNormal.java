package constantin.fpv_vr.APlay;
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

import com.google.vr.ndk.base.GvrLayout;
import com.google.vr.ndk.base.GvrUiLayout;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.GLRenderer.GLRStereoNormal;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.Toaster;
import constantin.renderingX.MyEGLConfigChooser;
import constantin.renderingX.PerformanceHelper;
import constantin.telemetry.core.TelemetryReceiver;

public class AStereoNormal extends AppCompatActivity {
    private GvrLayout mGvrLayout;
    private GLSurfaceView mGLViewStereo;

    private GLRStereoNormal mGLRStereoNormal;
    private AirHeadTrackingSender airHeadTrackingSender;
    private Context mContext;
    private TelemetryReceiver telemetryReceiver;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        mGvrLayout = new GvrLayout(this);
        final AStereoNormal instance=this;

        mGvrLayout.getUiLayout().setSettingsButtonListener(new Runnable() {
            @Override
            public void run() {
                GvrUiLayout.launchOrInstallGvrApp(instance);
                Toaster.makeToast(mContext,"Changing your vr viewer requires an activity restart",true);
            }
        });
        mGvrLayout.getUiLayout().setTransitionViewEnabled(false);
        mGvrLayout.setAsyncReprojectionEnabled(false);

        PerformanceHelper.enableSustainedPerformanceIfPossible(this);

        mGLViewStereo=new GLSurfaceView(this);
        mGLViewStereo.setEGLContextClientVersion(2);
        mGLViewStereo.setEGLConfigChooser(new MyEGLConfigChooser(SJ.DisableVSYNC(this),SJ.MultiSampleAntiAliasing(this)));
        //mGLViewStereo.setEGLWindowSurfaceFactory(new MyEGLWindowSurfaceFactory(true));
        telemetryReceiver=new TelemetryReceiver(this);
        mGLRStereoNormal = new GLRStereoNormal(this,telemetryReceiver, mGvrLayout.getGvrApi().getNativeGvrContext());


        mGLViewStereo.setRenderer(mGLRStereoNormal);
        mGLViewStereo.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLViewStereo.setPreserveEGLContextOnPause(true);
        mGvrLayout.setPresentationView(mGLViewStereo);
        setContentView(mGvrLayout);
        airHeadTrackingSender=new AirHeadTrackingSender(this,mGvrLayout.getGvrApi());

        registerForContextMenu(mGvrLayout);
    }


    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("YYY onResume()");
        PerformanceHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        PerformanceHelper.enableSustainedPerformanceIfPossible(this);
        telemetryReceiver.startReceiving();
        mGvrLayout.onResume();
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
        mGvrLayout.onPause();
        mGLViewStereo.onPause();
        PerformanceHelper.disableSustainedPerformanceIfEnabled(this);
    }


    @Override
    protected void onDestroy(){
        super.onDestroy();
        mGvrLayout.shutdown();
        mGvrLayout=null;
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
               mGvrLayout.getGvrApi().recenterTracking();
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }


}