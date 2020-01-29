package constantin.fpv_vr;

import android.content.Context;
import android.view.Surface;

import constantin.video.core.DecodingInfo;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.VideoPlayer;

public class MVideoPlayer implements IVideoParamsChanged {

    private final VideoPlayer videoPlayer;
    private final IVideoParamsChanged mVideoParamsChangedI;
    private final Surface mSurface;


    public MVideoPlayer(final Context context,final Surface surface, final IVideoParamsChanged vpc){
        mVideoParamsChangedI=vpc;
        mSurface=surface;
        videoPlayer=new VideoPlayer(context,this);
    }

    public synchronized void start(){
        videoPlayer.prepare(mSurface);
        videoPlayer.addAndStartReceiver();
    }

    public synchronized void stop(){
        videoPlayer.stopAndRemovePlayerReceiver();
    }

    //called by CPP code
    @Override
    public void onVideoRatioChanged(int videoW, int videoH) {
        if(mVideoParamsChangedI!=null){
            mVideoParamsChangedI.onVideoRatioChanged(videoW,videoH);
        }
    }

    @Override
    public void onDecodingInfoChanged(DecodingInfo decodingInfo) {
        if(mVideoParamsChangedI!=null){
            mVideoParamsChangedI.onDecodingInfoChanged(decodingInfo);
        }
    }

}
