package constantin.fpv_vr.XDJI;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import constantin.fpv_vr.Toaster;
import constantin.video.core.ISurfaceTextureAvailable;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer.VideoPlayer;
import constantin.video.core.VideoPlayer.VideoSettings;
import dji.sdk.base.BaseProduct;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.sdkmanager.DJISDKManager;

public class DJIVideoPlayerSurfaceTexture implements LifecycleObserver, ISurfaceTextureAvailable, VideoFeeder.VideoDataListener {
    //Used for Android lifecycle and executing callback on the UI thread
    private final AppCompatActivity parent;
    private final Context context;
    //null in the beginning, becomes valid in the future via onSurfaceTextureAvailable
    //(Constructor of Surface takes SurfaceTexture)
    private SurfaceTexture surfaceTexture;
    private Surface mVideoSurface;
    private DJICodecManager mCodecManager = null;
    private VideoPlayer videoPlayer;

    public DJIVideoPlayerSurfaceTexture(final AppCompatActivity parent){
        this.parent=parent;
        this.context=parent;
        VideoSettings.setVS_SOURCE(context, VideoSettings.VS_SOURCE.EXTERNAL);
        videoPlayer=new VideoPlayer(context,null);

        final BaseProduct product = DJISDKManager.getInstance().getProduct();
        if (product == null || !product.isConnected()) {
            Toaster.makeToast(context,"Cannot start video",true);
        } else {
            VideoFeeder.getInstance().getPrimaryVideoFeed().addVideoDataListener(this);
            Toaster.makeToast(context,"Start feeder",true);
        }
        parent.getLifecycle().addObserver(this);
    }


    public void setIVideoParamsChanged(final IVideoParamsChanged vpc){
        videoPlayer.setIVideoParamsChanged(vpc);
    }

    public long GetExternalGroundRecorder(){
        return 0;
    }

    //This one is called by the OpenGL Thread !
    @Override
    public void onSurfaceTextureAvailable(final SurfaceTexture surfaceTexture) {
        //To avoid race conditions always start and stop the Video player on the UI thread
        this.surfaceTexture=surfaceTexture;
        parent.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //If the callback gets called after the application was paused / destroyed
                //(which is possible because the callback was originally not invoked on the UI thread )
                //only create the Surface for later use. The next onResume() event will re-start the video
                mVideoSurface=new Surface(surfaceTexture);
                if(DJIVideoPlayerSurfaceTexture.this.parent.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
                    start();
                }
            }
        });
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        if(mVideoSurface!=null){
            start();
        }
    }

    private void start(){
        surfaceTexture.setDefaultBufferSize(1280,720);
        mCodecManager = new DJICodecManager(context,surfaceTexture,1280,720);
        System.out.println("Decoder okay ? "+mCodecManager.isDecoderOK()+" W H"+mCodecManager.getVideoWidth()+" "+mCodecManager.getVideoHeight());
        mCodecManager.cleanSurface();
        videoPlayer.addAndStartDecoderReceiver(mVideoSurface);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        if(mVideoSurface!=null){
            mCodecManager.destroyCodec();
            VideoFeeder.getInstance().getPrimaryVideoFeed().removeVideoDataListener(this);
            mCodecManager = null;
            videoPlayer.stopAndRemoveReceiverDecoder();
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void destroy(){
        if(mVideoSurface!=null){
            mVideoSurface.release();
        }
    }


    @Override
    public void onReceive(byte[] videoBuffer,int size) {
        //if (mCodecManager != null) {
        //    System.out.println("Data");
        //    mCodecManager.sendDataToDecoder(videoBuffer, size);
        //}
        VideoPlayer.nativePassNALUData(videoPlayer.getNativeInstance(),videoBuffer,0,size);
    }
}
