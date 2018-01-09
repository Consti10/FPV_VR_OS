package constantin.fpv_vr;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.graphics.SurfaceTexture;
import android.location.Location;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.os.Build;
import android.os.Process;
import android.view.Choreographer;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;

import com.google.vr.ndk.base.GvrApi;

import static android.content.Context.MODE_PRIVATE;


/**
 * Description: see Activity_StereoSuperSYNC
 */

public class GLRenderer14_StereoSuperSYNC implements GLSurfaceViewEGL14.IRendererEGL14,VideoPlayer.VideoParamsChanged,HomeLocation.HomeLocationChanged {
    static {
        System.loadLibrary("GLRSuperSync");
    }
    // Opaque native pointer to the native GLRendererStereo instance.
    public long nativeGLRSuperSync;
    private static native long nativeConstructRendererStereo(long nativeGvrContext);
    private static native long nativeDestroyRendererStereo(long glRendererStereoP);
    //surfaceCreated->surfaceChanged->drawFrame->pause->repeat
    private static native void nativeOnSurfaceCreated(long glRendererStereoP,int videoTexture,AssetManager assetManager,boolean qcomTiledRenderingAvailable);
    private static native void nativeOnSurfaceChanged(long glRendererStereoP,int width,int height);
    private static native void nativeOnDrawFrame(long glRendererStereoP,SurfaceTexture surfaceTexture,int exclusiveVRCore);
    private static native void nativeSetVideoDecoderFPS(long glRendererStereoP,float fps);
    private static native void nativeOnVideoRatioChanged(long glRendererStereoP,int videoW,int videoH);
    private static native void nativeDoFrame(long glRendererStereoP,long lastVsync);
    private static native void nativeOnGLSurfaceDestroyed(long glRendererStereoP);
    public static native void nativeExitSuperSyncLoop(long glRendererMonoP);
    private static native void nativeSetHomeLocation(long glRendererStereoP,double latitude, double longitude,double attitude);

    private final Context mContext;
    private SurfaceTexture mSurfaceTexture;
    private VideoPlayer mVideoPlayer;

    private final long choreographerVsyncOffsetNS;


    public GLRenderer14_StereoSuperSYNC(Context context, long gvrApiNativeContext){
        mContext=context;
        nativeGLRSuperSync=nativeConstructRendererStereo(gvrApiNativeContext);
        Display d=((WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        float refreshRating=d.getRefreshRate();
        System.out.println("refreshRate:"+refreshRating);
        choreographerVsyncOffsetNS=d.getAppVsyncOffsetNanos();
        System.out.println("app Vsync offset"+choreographerVsyncOffsetNS/1000000.0f);
    }

    @Override
    public void onSurfaceCreated() {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        mSurfaceTexture = new SurfaceTexture(videoTexture[0],false);
        Surface surface=new Surface(mSurfaceTexture);
        mVideoPlayer=VideoPlayer.createAndStartVideoPlayer(mContext,surface,this);
        AssetManager assetManager=mContext.getAssets();
        SharedPreferences pref_static = mContext.getSharedPreferences("pref_static", MODE_PRIVATE);
        boolean qcomTiledRenderingAvailable=pref_static.getBoolean(mContext.getString(R.string.GL_QCOM_tiled_renderingAvailable),false);
        nativeOnSurfaceCreated(nativeGLRSuperSync,videoTexture[0],assetManager,qcomTiledRenderingAvailable);
    }

    @Override
    public void onSurfaceChanged(int width, int height){
        nativeOnSurfaceChanged(nativeGLRSuperSync,width,height);
    }

    @Override
    public void onDrawFrame() {
        //Clear and swap once
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT|GLES20.GL_COLOR_BUFFER_BIT|GLES20.GL_STENCIL_BUFFER_BIT);
        EGL14.eglSwapBuffers(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
        Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
        Process.setThreadPriority(-20);
        int exclusiveVRCore=2; //use the 3rd core default
        if (Build.VERSION.SDK_INT >= 24) {
            int[] cores= Process.getExclusiveCores();
            if(cores!=null){
                if(cores.length>=1){
                    exclusiveVRCore=cores[0];
                }
            }
        }
        nativeOnDrawFrame(nativeGLRSuperSync,mSurfaceTexture,exclusiveVRCore);
    }

    @Override
    public void onGLSurfaceDestroyed() {
        //System.out.println("GL14 onDestroy");
        if(mVideoPlayer!=null){
            VideoPlayer.stopAndDeleteVideoPlayer(mVideoPlayer);
            mVideoPlayer=null;
        }
        if(mSurfaceTexture!=null){
            mSurfaceTexture.release();
            mSurfaceTexture=null;
        }
        nativeOnGLSurfaceDestroyed(nativeGLRSuperSync);
    }

    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {
        nativeOnVideoRatioChanged(nativeGLRSuperSync,videoW,videoH);
    }

    @Override
    public void onVideoFPSChanged(float decFPS) {
        nativeSetVideoDecoderFPS(nativeGLRSuperSync,decFPS);
    }

    //@Override
    public void doFramePasstrough(long frameTimeNanos) {
        //####google SurfaceFlinger.cpp ##################
        // We add an additional 1ms to allow for processing time and
        // differences between the ideal and actual refresh rate.
        nativeDoFrame(nativeGLRSuperSync,frameTimeNanos-choreographerVsyncOffsetNS+1000000);
    }

    @Override
    protected void finalize() throws Throwable {
        //System.out.println("Hi finalize");
        //TODO: is this a good practice ? maybe take a deeper look. But it 'works'
        try {
            nativeDestroyRendererStereo(nativeGLRSuperSync);
        } finally {
            super.finalize();
        }
    }

    @Override
    public void onHomeLocationChanged(Location location) {
        nativeSetHomeLocation(nativeGLRSuperSync,location.getLatitude(),location.getLongitude(),location.getAltitude());
    }
}

