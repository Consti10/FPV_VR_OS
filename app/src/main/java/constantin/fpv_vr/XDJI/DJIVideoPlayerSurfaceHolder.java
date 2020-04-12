package constantin.fpv_vr.XDJI;

import android.content.Context;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import constantin.fpv_vr.Toaster;
import constantin.video.core.IVideoParamsChanged;
import dji.common.product.Model;
import dji.sdk.base.BaseProduct;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.sdkmanager.DJISDKManager;

public class DJIVideoPlayerSurfaceHolder implements SurfaceHolder.Callback {
    private final VideoFeeder.VideoDataListener mReceivedVideoDataListener;
    private DJICodecManager mCodecManager = null;
    private final Context context;
    private IVideoParamsChanged iVideoParamsChanged;

    public DJIVideoPlayerSurfaceHolder(final Context context,final SurfaceView surfaceView){
        this.context=context;
        mReceivedVideoDataListener = new VideoFeeder.VideoDataListener() {
            @Override
            public void onReceive(byte[] videoBuffer, int size) {
                if (mCodecManager != null) {
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
        surfaceView.getHolder().addCallback(this);
    }
    public void setIVideoParamsChanged(final IVideoParamsChanged vpc){
        this.iVideoParamsChanged=vpc;
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
            mCodecManager = new DJICodecManager(context,holder,width,height);
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

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mCodecManager != null) {
            mCodecManager.cleanSurface();
            mCodecManager = null;
        }
    }
}
