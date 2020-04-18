package constantin.fpv_vr.xdji;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import constantin.fpv_vr.Toaster;
import constantin.video.core.video_player.VideoPlayer;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.products.Aircraft;

// Use either one of the Interfaces to start() / stop the video player
// e.g use either SurfaceView.getHolder().addCallback(videoPlayer); or
// use new VideoSurfaceHolder(context,videoPlayer);

public class DJIVideoPlayer extends VideoPlayer {
    private final boolean DJI_ENABLED;
    private DJICodecManager mCodecManager;
    private final Context context;

    public DJIVideoPlayer(Context context) {
        super(context,null);
        this.context=context;
        DJI_ENABLED=DJIApplication.isDJIEnabled(context);
        if(DJI_ENABLED){
            final Aircraft aircraft=DJIApplication.getConnectedAircraft();
            if (aircraft==null) {
                Toaster.makeToast(context,"Cannot start video",true);
            } else {
                VideoFeeder.getInstance().getPrimaryVideoFeed().addVideoDataListener(this::onReceiveDjiData);
                Toaster.makeToast(context,"Start feeder",true);
            }
        }
    }

    // Order is important !
    @Override
    public void addAndStartDecoderReceiver(Surface surface){
        if(DJI_ENABLED){
            mCodecManager=new DJICodecManager(context,new SurfaceTexture(0),1280,720);
            System.out.println("Decoder okay ? "+mCodecManager.isDecoderOK()+" W H"+mCodecManager.getVideoWidth()+" "+mCodecManager.getVideoHeight());
            mCodecManager.cleanSurface();
        }
        super.addAndStartDecoderReceiver(surface);
    }
    // Order is important !
    @Override
    public void stopAndRemoveReceiverDecoder(){
        if(DJI_ENABLED){
            mCodecManager.destroyCodec();
            VideoFeeder.getInstance().getPrimaryVideoFeed().removeVideoDataListener(this::onReceiveDjiData);
            mCodecManager = null;
        }
        super.stopAndRemoveReceiverDecoder();
    }


    private void onReceiveDjiData(byte[] videoBuffer,int size) {
        //if (mCodecManager != null) {
        //    System.out.println("Data");
        //    mCodecManager.sendDataToDecoder(videoBuffer, size);
        //}
        VideoPlayer.nativePassNALUData(getNativeInstance(),videoBuffer,0,size);
    }

}
