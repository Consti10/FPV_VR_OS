package constantin.fpv_vr.xdji;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.SurfaceHolder;

import constantin.fpv_vr.Toaster;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.video_player.VideoPlayer;
import constantin.video.core.video_player.VideoSettings;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.products.Aircraft;

// Do not forget to set the callback
public class XVideoPlayerSurfaceHolder  implements SurfaceHolder.Callback{
    private DJICodecManager mCodecManager = null;
    private final Context context;
    private VideoPlayer videoPlayer;
    private final boolean DJI_ENABLED;

    public XVideoPlayerSurfaceHolder(final Context context){
        this.context=context;
        DJI_ENABLED=DJIApplication.isDJIEnabled(context);
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
    public void surfaceCreated(SurfaceHolder holder) {
        //Create a fake DJICodecManager to receive live video data
        if(DJI_ENABLED){
            mCodecManager=new DJICodecManager(context,new SurfaceTexture(0),1280,720);
            mCodecManager.cleanSurface();
        }
        videoPlayer.addAndStartDecoderReceiver(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if(DJI_ENABLED){
            mCodecManager.destroyCodec();
            VideoFeeder.getInstance().getPrimaryVideoFeed().removeVideoDataListener(this::onReceiveDjiData);
            mCodecManager = null;
        }
        videoPlayer.stopAndRemoveReceiverDecoder();
    }

    private void onReceiveDjiData(byte[] videoBuffer,int size) {
        //if (mCodecManager != null) {
        //    System.out.println("Data");
        //    mCodecManager.sendDataToDecoder(videoBuffer, size);
        //}
        VideoPlayer.nativePassNALUData(videoPlayer.getNativeInstance(),videoBuffer,0,size);
    }
}
