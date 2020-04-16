package constantin.fpv_vr.xdji;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import constantin.fpv_vr.Toaster;
import constantin.renderingx.core.video.ISurfaceAvailable;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.video_player.VideoPlayer;
import constantin.video.core.video_player.VideoSettings;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.products.Aircraft;

// Do not forget to set the callback
public class XVideoPlayerSurfaceTexture implements ISurfaceAvailable {
    private final VideoPlayer videoPlayer;
    private final Context context;
    private final boolean DJI_ENABLED;
    private DJICodecManager mCodecManager;

    public XVideoPlayerSurfaceTexture(final Context context){
        this.context=context;
        DJI_ENABLED=DJIApplication.isDJIEnabled(context);
        if(DJI_ENABLED){
            VideoSettings.setVS_SOURCE(context, VideoSettings.VS_SOURCE.EXTERNAL);
        }
        videoPlayer=new VideoPlayer(context,null);
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

    public void setIVideoParamsChanged(final IVideoParamsChanged vpc){
        videoPlayer.setIVideoParamsChanged(vpc);
    }

    public long getExternalGroundRecorder(){
        return videoPlayer.getExternalGroundRecorder();
    }

    @Override
    public void start(SurfaceTexture surfaceTexture, Surface surface) {
        if(DJI_ENABLED){
            surfaceTexture.setDefaultBufferSize(1280,720);
            mCodecManager = new DJICodecManager(context,surfaceTexture,1280,720);
            System.out.println("Decoder okay ? "+mCodecManager.isDecoderOK()+" W H"+mCodecManager.getVideoWidth()+" "+mCodecManager.getVideoHeight());
            mCodecManager.cleanSurface();
        }
        videoPlayer.addAndStartDecoderReceiver(surface);
    }

    @Override
    public void stop() {
        videoPlayer.stopAndRemoveReceiverDecoder();
        if(DJI_ENABLED){
            mCodecManager.destroyCodec();
            VideoFeeder.getInstance().getPrimaryVideoFeed().removeVideoDataListener(this::onReceiveDjiData);
            mCodecManager = null;
        }
    }

    private void onReceiveDjiData(byte[] videoBuffer,int size) {
        //if (mCodecManager != null) {
        //    System.out.println("Data");
        //    mCodecManager.sendDataToDecoder(videoBuffer, size);
        //}
        VideoPlayer.nativePassNALUData(this.videoPlayer.getNativeInstance(),videoBuffer,0,size);
    }
}
