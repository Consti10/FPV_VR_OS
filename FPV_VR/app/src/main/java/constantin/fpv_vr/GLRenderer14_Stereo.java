package constantin.fpv_vr;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLExt;
import android.opengl.GLES20;
import android.view.Surface;
import com.google.vr.ndk.base.GvrApi;

/**
 * Description: see Activity_Stereo
 */

public class GLRenderer14_Stereo implements GLSurfaceViewEGL14.IRendererEGL14,VideoPlayerInterface{
    static {
        System.loadLibrary("GLRendererStereo");
    }
    // Opaque native pointer to the native GLRendererStereo instance.
    private long nativeGLRendererStereo;
    private static native long nativeConstructRendererStereo(long nativeGvrContext);
    private static native long nativeDestroyRendererStereo(long glRendererStereoP);
    //surfaceCreated->surfaceChanged->drawFrame->GLSurfaceDestroyed->repeat
    public static native void nativeOnSurfaceCreated(long glRendererStereoP,int videoTexture,AssetManager assetManager);
    public static native void nativeOnSurfaceChanged(long glRendererStereoP,int width,int height);
    public static native void nativeOnDrawFrame(long glRendererStereoP);
    private static native void nativeSetVideoDecoderFPS(long glRendererStereoP,float fps);
    private static native void nativeOnVideoRatioChanged(long glRendererStereoP,int videoW,int videoH);
    //since we do not preserve the OpenGL context when paused, nativeOnSurfaceCreated acts as a "onResume" equivalent.
    //private static native void nativeOnPause(long glRendererStereoP);
    private static native void nativeOnGLSurfaceDestroyed(long glRendererStereoP);

    private final Context mContext;
    private SurfaceTexture mSurfaceTexture=null;
    private VideoPlayer mVideoPlayer=null;


    public GLRenderer14_Stereo(Context activityContext,long gvrApiNativeContext){
        mContext=activityContext;
        nativeGLRendererStereo=nativeConstructRendererStereo(
                gvrApiNativeContext);
    }

    @Override
    public void onSurfaceCreated() {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        mSurfaceTexture = new SurfaceTexture(videoTexture[0],false);
        Surface mVideoSurface=new Surface(mSurfaceTexture);
        mVideoPlayer=VideoPlayer.createAndStartVideoPlayer(mContext,mVideoSurface,this);
        AssetManager assetManager=mContext.getAssets();
        nativeOnSurfaceCreated(nativeGLRendererStereo,videoTexture[0],assetManager);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        nativeOnSurfaceChanged(nativeGLRendererStereo,width,height);
    }

    @Override
    public void onDrawFrame() {
        if(Settings.Disable60fpsCap){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
        }
        mSurfaceTexture.updateTexImage();
        if(Settings.Disable60fpsCap){
            EGLExt.eglPresentationTimeANDROID(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW),System.nanoTime());
        }
        nativeOnDrawFrame(nativeGLRendererStereo);
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
        nativeOnGLSurfaceDestroyed(nativeGLRendererStereo);
    }

    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {
        nativeOnVideoRatioChanged(nativeGLRendererStereo,videoW,videoH);
    }

    @Override
    public void onVideoFPSChanged(float decFPS) {
        nativeSetVideoDecoderFPS(nativeGLRendererStereo,decFPS);
    }

    @Override
    protected void finalize() throws Throwable {
        //System.out.println("Hi finalize");
        //TODO: is this a good practice ? maybe take a deeper look. But it 'works'
        try {
            nativeDestroyRendererStereo(nativeGLRendererStereo);
        } finally {
            super.finalize();
        }
    }


}
