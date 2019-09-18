package constantin.fpv_vr.GLRenderer;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.google.vr.ndk.base.GvrApi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.fpv_vr.MVideoPlayer;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;

/**
 * Renders OSD only
 */

public class GLRMono360 implements GLSurfaceView.Renderer, IVideoParamsChanged {
    static {
        System.loadLibrary("GLRMono360");
    }
    private native long nativeConstruct(Context context,long telemetryReceiver,long nativeGvrContext,boolean renderOSD);
    private native void nativeDelete(long glRendererMonoP);
    private native void nativeOnSurfaceCreated(long glRendererP,int videoTexture,Context androidContext);
    private native void nativeOnSurfaceChanged(long glRendererMonoP,int width,int height);
    private native void nativeOnDrawFrame(long glRendererMonoP);

    private final long nativeGLRendererMono;
    private final Context mContext;
    private SurfaceTexture mSurfaceTexture;
    private MVideoPlayer mVideoPlayer;

    public GLRMono360(final Context context, final TelemetryReceiver telemetryReceiver, GvrApi gvrApi,final boolean renderOSD){
        mContext=context;
        nativeGLRendererMono=nativeConstruct(context,telemetryReceiver.getNativeInstance(),gvrApi.getNativeGvrContext(),renderOSD);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        int[] videoTexture=new int[1];
        GLES20.glGenTextures(1, videoTexture, 0);
        final int mGLTextureVideo = videoTexture[0];
        mSurfaceTexture = new SurfaceTexture(mGLTextureVideo,false);
        nativeOnSurfaceCreated(nativeGLRendererMono,mGLTextureVideo,mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeGLRendererMono,width,height);
        //MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        startVideoPlayerIfNotAlreadyRunning();
        if(mSurfaceTexture!=null){
            mSurfaceTexture.updateTexImage();
        }
        nativeOnDrawFrame(nativeGLRendererMono);
    }

    public void onPause(){
        System.out.println("onPause()");
        if(mVideoPlayer!=null){
            mVideoPlayer.stop();
            mVideoPlayer=null;
        }
    }

    private void startVideoPlayerIfNotAlreadyRunning(){
        if(mVideoPlayer==null){
            Surface mVideoSurface=new Surface(mSurfaceTexture);
            mVideoPlayer=new MVideoPlayer(mContext,mVideoSurface,this);
            mVideoPlayer.start();
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeGLRendererMono);
        } finally {
            super.finalize();
        }
    }

    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {

    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {

    }
}
