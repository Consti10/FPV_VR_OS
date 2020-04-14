package constantin.fpv_vr.XDJI;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import constantin.fpv_vr.Toaster;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer.VideoPlayer;
import constantin.video.core.VideoPlayer.VideoSettings;
import dji.common.product.Model;
import dji.midware.usb.P3.UsbAccessoryService;
import dji.sdk.base.BaseProduct;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.sdkmanager.DJISDKManager;

public class DJIVideoPlayerSurfaceHolder implements SurfaceHolder.Callback ,VideoFeeder.VideoDataListener{
    private DJICodecManager mCodecManager = null;
    private final Context context;
    private IVideoParamsChanged iVideoParamsChanged;
    private VideoPlayer videoPlayer;

    public DJIVideoPlayerSurfaceHolder(final Context context,final SurfaceView surfaceView){
        this.context=context;
        VideoSettings.setVS_SOURCE(context, VideoPlayer.VS_SOURCE.EXTERNAL);
        videoPlayer=new VideoPlayer(context,null);

        final BaseProduct product = DJISDKManager.getInstance().getProduct();
        if (product == null || !product.isConnected()) {
            Toaster.makeToast(context,"Cannot start video",true);
        } else {
            VideoFeeder.getInstance().getPrimaryVideoFeed().addVideoDataListener(this);
            Toaster.makeToast(context,"Start feeder",true);
        }
        surfaceView.getHolder().addCallback(this);
    }
    public void setIVideoParamsChanged(final IVideoParamsChanged vpc){
        videoPlayer.setIVideoParamsChanged(vpc);
    }
    public long GetExternalGroundRecorder(){
        return 0;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if(mCodecManager==null){
            System.out.println("W H"+width+" "+height);
            //mCodecManager = new DJICodecManager(context,holder,width,height);
            mCodecManager=new DJICodecManager(context,new SurfaceTexture(0),1280,720);
            //mCodecManager.switchSource(DJICodecManager.VideoSource.FPV);
            mCodecManager.setOnVideoSizeChangedListener(new DJICodecManager.OnVideoSizeChangedListener() {
                @Override
                public void onVideoSizeChanged(int w, int h) {
                    if(iVideoParamsChanged!=null){
                        iVideoParamsChanged.onVideoRatioChanged(w,h);
                        Toaster.makeToast(context,"Video w h"+w+" "+h,false);
                    }
                }
            });
            mCodecManager.cleanSurface();
            videoPlayer.addAndStartDecoderReceiver(holder.getSurface());
            //mCodecManager.cleanSurface();
            //mCodecManager.destroyCodec();
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mCodecManager != null) {
            mCodecManager.destroyCodec();
            VideoFeeder.getInstance().getPrimaryVideoFeed().removeVideoDataListener(this);
            mCodecManager = null;
        }
        videoPlayer.stopAndRemoveReceiverDecoder();
    }

    @Override
    public void onReceive(byte[] videoBuffer, int size) {
        System.out.println("Data arrived");
        if (mCodecManager != null) {
            mCodecManager.sendDataToDecoder(videoBuffer, size);
        }
        VideoPlayer.nativePassNALUData(videoPlayer.getNativeInstance(),videoBuffer,0,size);
    }
}
