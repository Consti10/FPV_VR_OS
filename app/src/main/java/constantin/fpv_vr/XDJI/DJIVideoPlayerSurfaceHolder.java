package constantin.fpv_vr.XDJI;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import constantin.fpv_vr.Toaster;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer.VideoPlayer;
import constantin.video.core.VideoPlayer.VideoSettings;
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
        VideoSettings.setVS_SOURCE(context, VideoSettings.VS_SOURCE.EXTERNAL);
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
        //Create a fake DJICodecManager to receive live video data
        mCodecManager=new DJICodecManager(context,new SurfaceTexture(0),1280,720);
        mCodecManager.cleanSurface();
        videoPlayer.addAndStartDecoderReceiver(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mCodecManager.destroyCodec();
        VideoFeeder.getInstance().getPrimaryVideoFeed().removeVideoDataListener(this);
        mCodecManager = null;
        videoPlayer.stopAndRemoveReceiverDecoder();
    }

    @Override
    public void onReceive(byte[] videoBuffer, int size) {
        //System.out.println("Data arrived");
        VideoPlayer.nativePassNALUData(videoPlayer.getNativeInstance(),videoBuffer,0,size);
    }
}

//if (mCodecManager != null) {
//    mCodecManager.sendDataToDecoder(videoBuffer, size);
//}
//mCodecManager.setOnVideoSizeChangedListener(new DJICodecManager.OnVideoSizeChangedListener() {
//                @Override
//                public void onVideoSizeChanged(int w, int h) {
//                    if(iVideoParamsChanged!=null){
//                        iVideoParamsChanged.onVideoRatioChanged(w,h);
//                        Toaster.makeToast(context,"Video w h"+w+" "+h,false);
//                    }
//                }
//            });