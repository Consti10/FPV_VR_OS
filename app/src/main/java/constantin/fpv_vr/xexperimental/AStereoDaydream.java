package constantin.fpv_vr.xexperimental;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrLayout;

import constantin.telemetry.core.TelemetryReceiver;

/**
 * Uses the Android daydream video plugin
 * This activity can only be used on Daydream-ready devices and has slightly more latency than SuperSYNC
 * But corrects for chromatic aberration on the other side
 * Pipeline h.264-->image on screen:
 * h.264 NALUs->VideoDecoder->GvrApi video texture (maybe external texture ? unclear)->Rendering with OpenGL in the async timewarp thread (by GvrApi)
 */

public class AStereoDaydream extends AppCompatActivity implements GvrLayout.ExternalSurfaceListener{
    private GvrLayout mGvrLayout;
    private GLSurfaceView mGLView;
    private GLRStereoDaydream mGLRStereoDayDream;
    private Context mContext;
    private TelemetryReceiver telemetryReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        //AndroidCompat.setVrModeEnabled(this, true);
        //AndroidCompat.setSustainedPerformanceMode(this,true);

        mGvrLayout = new GvrLayout(this);
        mGvrLayout.setKeepScreenOn(true);
        //mGvrLayout.getUiLayout().setTransitionViewEnabled(false);
        //mGvrLayout.setAsyncReprojectionEnabled(false);
        mGvrLayout.setAsyncReprojectionEnabled(false);
        //mGvrLayout.enableAsyncReprojectionVideoSurface(this,new Handler(Looper.getMainLooper()),false);

        mGLView = new GLSurfaceView(this);
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(8, 8, 8, 0, 0, 0);
        mGLView.setPreserveEGLContextOnPause(true);
        telemetryReceiver=new TelemetryReceiver(this,0,0);
        mGLRStereoDayDream =new GLRStereoDaydream(this,telemetryReceiver, mGvrLayout.getGvrApi(),0);//mGvrLayout.getAsyncReprojectionVideoSurfaceId()
        mGLView.setRenderer(mGLRStereoDayDream);
        mGvrLayout.setPresentationView(mGLView);
        setContentView(mGvrLayout);
    }

    @Override
    protected void onResume(){
        super.onResume();
        //System.out.println("YYY onResume()");
        mGvrLayout.onResume();
        mGLView.onResume();
    }

    @Override
    protected void onPause(){
        //System.out.println("YYY onPause()");
        synchronized (this){
        }
        mGvrLayout.onPause();
        mGLView.onPause();
        super.onPause();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        mGvrLayout.shutdown();
        mGvrLayout=null;
        mGLView =null;
        mGLRStereoDayDream=null;
    }

    @Override
    public void onSurfaceAvailable(Surface surface) {
        //System.out.println("Native video surface avaialable");
        /*Canvas c=surface.lockCanvas(null);
        c.drawColor(Color.RED);
        surface.unlockCanvasAndPost(c);*/
        synchronized (this){

        }
    }

    @Override
    public void onFrameAvailable() {
    }
}
