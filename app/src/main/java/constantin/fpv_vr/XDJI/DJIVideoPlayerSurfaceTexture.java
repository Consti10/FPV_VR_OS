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
import dji.common.product.Model;
import dji.sdk.base.BaseProduct;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.sdkmanager.DJISDKManager;

public class DJIVideoPlayerSurfaceTexture implements LifecycleObserver, ISurfaceTextureAvailable {
    //Used for Android lifecycle and executing callback on the UI thread
    private final AppCompatActivity parent;
    private final Context context;
    //null in the beginning, becomes valid in the future via onSurfaceTextureAvailable
    //(Constructor of Surface takes SurfaceTexture)
    private SurfaceTexture surfaceTexture;
    private final VideoFeeder.VideoDataListener mReceivedVideoDataListener;
    private DJICodecManager mCodecManager = null;
    private IVideoParamsChanged iVideoParamsChanged;

    public DJIVideoPlayerSurfaceTexture(final AppCompatActivity parent){
        this.parent=parent;
        this.context=parent;
        //videoPlayer=new VideoPlayer(parent,null);
        mReceivedVideoDataListener = new VideoFeeder.VideoDataListener() {
            @Override
            public void onReceive(byte[] videoBuffer, int size) {
                if (mCodecManager != null) {
                    System.out.println("Data");
                    mCodecManager.sendDataToDecoder(videoBuffer, size);
                }
            }
        };
        final BaseProduct product = DJISDKManager.getInstance().getProduct();
        if (product == null || !product.isConnected()) {
            Toaster.makeToast(context,"Cannot start video",true);
        } else {
            if (!product.getModel().equals(Model.UNKNOWN_AIRCRAFT)) {
                VideoFeeder.getInstance().getPrimaryVideoFeed().addVideoDataListener(mReceivedVideoDataListener);
                Toaster.makeToast(context,"Start feeder",true);
            }else{
                Toaster.makeToast(context,"Unknown aircraft",true);
            }
        }
        parent.getLifecycle().addObserver(this);
    }


    public void setIVideoParamsChanged(final IVideoParamsChanged vpc){
        this.iVideoParamsChanged=vpc;
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
                //mVideoSurface=new Surface(surfaceTexture);
                if(DJIVideoPlayerSurfaceTexture.this.parent.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
                    if(mCodecManager==null){
                        surfaceTexture.setDefaultBufferSize(1280,720);
                        mCodecManager = new DJICodecManager(context,surfaceTexture,1280,720);
                        System.out.println("Decoder okay ? "+mCodecManager.isDecoderOK()+" W H"+mCodecManager.getVideoWidth()+" "+mCodecManager.getVideoHeight());
                        mCodecManager.setOnVideoSizeChangedListener(new DJICodecManager.OnVideoSizeChangedListener() {
                            @Override
                            public void onVideoSizeChanged(int w, int h) {
                                if(iVideoParamsChanged!=null){
                                    iVideoParamsChanged.onVideoRatioChanged(w,h);
                                    Toaster.makeToast(context,"Video w h"+w+" "+h,false);
                                }
                            }
                        });
                    }
                }
            }
        });
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        if(surfaceTexture!=null){
            if(mCodecManager==null){
                surfaceTexture.setDefaultBufferSize(1280,720);
                mCodecManager = new DJICodecManager(context,surfaceTexture,1280,720);
                System.out.println("Decoder okay ? "+mCodecManager.isDecoderOK()+" W H"+mCodecManager.getVideoWidth()+" "+mCodecManager.getVideoHeight());
            }
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        if(surfaceTexture!=null){
            if (mCodecManager != null) {
                mCodecManager.cleanSurface();
                mCodecManager = null;
            }
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void destroy(){
        //if(mVideoSurface!=null){
        //    mVideoSurface.release();
        //}
    }


}
