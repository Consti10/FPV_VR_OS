package constantin.fpv_vr.xdji;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.Nullable;

import constantin.fpv_vr.Toaster;
import constantin.renderingx.core.video.ISurfaceAvailable;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.video_player.VideoPlayer;
import dji.sdk.camera.VideoFeeder;
import dji.sdk.codec.DJICodecManager;
import dji.sdk.products.Aircraft;

// Use either one of the Interfaces to start() / stop the video player
// e.g use either SurfaceView.getHolder().addCallback(videoPlayer); or
// use new VideoSurfaceHolder(context,videoPlayer);
public class XVideoPlayer implements SurfaceHolder.Callback, ISurfaceAvailable {
    private final VideoPlayer videoPlayer;
    private final Context context;
    private final boolean DJI_ENABLED;
    private DJICodecManager mCodecManager;

    public XVideoPlayer(final Context context){
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
    public long getExternalFileReader(){
        return videoPlayer.getExternalFilePlayer();
    }

    // Called when configured with SurfaceHolder
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        startPlayer(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPlayer();
    }

    // Called when configured with ISurfaceAvailable
    @Override
    public void XSurfaceCreated(SurfaceTexture surfaceTexture, Surface surface) {
        startPlayer(surface);
    }

    @Override
    public void XSurfaceDestroyed() {
        stopPlayer();
    }

    private void startPlayer(Surface surface){
        if(DJI_ENABLED){
           mCodecManager=new DJICodecManager(context,new SurfaceTexture(0),1280,720);
            System.out.println("Decoder okay ? "+mCodecManager.isDecoderOK()+" W H"+mCodecManager.getVideoWidth()+" "+mCodecManager.getVideoHeight());
            mCodecManager.cleanSurface();
        }
        videoPlayer.addAndStartDecoderReceiver(surface);
    }
    private void stopPlayer(){
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
        VideoPlayer.nativePassNALUData(this.videoPlayer.getNativeInstance(),videoBuffer,0,size);
    }
}
