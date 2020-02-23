package constantin.fpv_vr.PlayStereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;

import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

import com.google.vr.ndk.base.BufferViewport;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.renderingx.core.ISurfaceTextureAvailable;
import constantin.fpv_vr.MVideoPlayer;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.renderingx.core.FullscreenHelper;
import constantin.renderingx.core.MyEGLConfigChooser;
import constantin.renderingx.core.MyVRLayout;
import constantin.telemetry.core.TelemetryReceiver;

public class AStereoNormal extends AppCompatActivity implements ISurfaceTextureAvailable {
    //private GvrLayout mVrLayout;
    private MyVRLayout mVrLayout;
    private GLSurfaceView mGLViewStereo;

    private GLRStereoNormal mGLRStereoNormal;
    private AirHeadTrackingSender airHeadTrackingSender;
    private Context mContext;
    private TelemetryReceiver telemetryReceiver;
    private MVideoPlayer mVideoPlayer;
    private SurfaceTexture surfaceTexture;

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
        mGLRStereoNormal = new GLRStereoNormal(this,this,telemetryReceiver, mVrLayout.getGvrApi().getNativeGvrContext());

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

    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("YYY onResume()");
        FullscreenHelper.setImmersiveSticky(this);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mGLViewStereo.onResume();
    }


    @Override
    protected void onPause(){
        super.onPause();
        System.out.println("YYY onPause()");
        mGLViewStereo.onPause();
        if(mVideoPlayer!=null){
            mVideoPlayer.stop();
            mVideoPlayer=null;
        }
    }


    @Override
    public void onSurfaceTextureAvailable(final SurfaceTexture surfaceTexture) {
        //Set the surface texture to 'available' on the GL Thread !
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ((AStereoNormal)mContext).surfaceTexture=surfaceTexture;
                if(mVideoPlayer==null){
                    Surface mVideoSurface=new Surface(surfaceTexture);
                    mVideoPlayer=new MVideoPlayer(mContext,mVideoSurface,mGLRStereoNormal);
                    mVideoPlayer.start();
                }
            }
        });
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mVrLayout =null;
        mGLViewStereo =null;
        mGLRStereoNormal=null;
        airHeadTrackingSender=null;
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