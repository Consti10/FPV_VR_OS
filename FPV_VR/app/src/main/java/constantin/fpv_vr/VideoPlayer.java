package constantin.fpv_vr;
/* ********************************************************************************
 * Since we have a video stream as data source, this player does not support "pausing"
 *
 * Warning: Don't forget to call stopAndDeleteVideoPlayer() after createAndStartVideoPlayer()
 * Else some resources may not be freed, which can result in crash/ unexpected behaviour
 *
 * Usage:
 * a) create a instance with a valid surface via createAndStartVideoPlayer()
 *    --> the player will start as soon as he has received enough h.264 configuration NALU's.
 *      With wifibroadcast, the stream might contain incorrect NALU's ->but we feed them anyway. I have
 *      not seen a decoder failing because of this, only creating "bad pixel blocks" until a new I-frame is received
 * b) when the video surface has to be deleted (e.g. the app is paused), call
 *    stopAndDeleteVideoPlayer(). This will delete the LowLag decoder and free resources.
 *    after stopAndDeleteVideoPlayer the instance is no longer usable
 *
 * A java VideoPlayer Instance holds up to two cpp instances:
 *  1) one LowLagDecoder instance
 *  2) one UDPReceiver instance, if the Data Source is UDP
 **********************************************************************************/

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import android.view.Surface;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;


public class VideoPlayer {
    static {
        System.loadLibrary("VideoPlayerN");
    }
    private native void createDecoder(boolean limitFPS,Surface surface);
    //private native void shutdownDecoder();
    private native void requestShutdown();
    private native void waitAndDelete();
    private native void passNALUDataToNative(byte[] b,int l);

    private native void createUDPReceiver(int port);
    private native void startReceiving();
    private native void stopReceiving();

    private Context mContext;
    private Thread receiverThread;
    private VideoParamsChanged mVideoParamsChangedI;
    private final Object lock=new Object();

    private static final int BUFER_SIZE=1024*1024;

    private final int mConnectionType;
    //private final String videoFromAssetsFileName="example.h264";
    private static final String videoFromAssetsFileName="video.h264";
    private final String videoFromIntStorageFileName;


    public static VideoPlayer createAndStartVideoPlayer(Context context, Surface surface, VideoParamsChanged vri){
        VideoPlayer videoPlayer=new VideoPlayer(context, surface, vri);
        videoPlayer.startPlaying();
        return videoPlayer;
    }

    /**
     * After stopAndDeleteVideoPlayer, the VideoPlayer instance currently existing in the calling super class
     * should be null-ed to make it deletable by the garbage collector
     * @param videoPlayer: a VideoPlayer instance, previously created via createAndStartVideoPlayer()
     */
    public static void stopAndDeleteVideoPlayer(VideoPlayer videoPlayer){
        videoPlayer.stopPlaying();
    }

    private VideoPlayer(Context context, Surface surface, VideoParamsChanged vri){
        mContext=context;
        mConnectionType=Settings.ConnectionType;
        videoFromIntStorageFileName=Settings.FilenameVideo;
        boolean limitFPS=false;
        if(mConnectionType==Settings.ConnectionTypeTestFile ||mConnectionType==Settings.ConnectionTypeStorageFile){
            limitFPS=true;
        }
        createDecoder(limitFPS,surface);
        mVideoParamsChangedI=vri;
    }

    private void startPlaying(){
        synchronized (lock){
            switch (mConnectionType){
                case Settings.ConnectionTypeEZWB:
                    createUDPReceiver(Settings.UDPPortVideo);
                    startReceiving();
                    break;
                 case Settings.ConnectionTypeManually:
                    createUDPReceiver(Settings.UDPPortVideo);
                    startReceiving();
                    break;
                case Settings.ConnectionTypeTestFile:
                    receiverThread=new Thread() {
                        @Override
                        public void run() {
                            receiveFromAssets();
                        }
                    };
                    receiverThread.setName("AssetsRecT");
                    receiverThread.setPriority(Thread.NORM_PRIORITY);
                    receiverThread.start();
                    break;
                case Settings.ConnectionTypeStorageFile:
                    receiverThread=new Thread() {
                        @Override
                        public void run() {
                            receiveFromFile(videoFromIntStorageFileName);
                        }
                    };
                    receiverThread.setName("FileRecT");
                    receiverThread.setPriority(Thread.NORM_PRIORITY);
                    receiverThread.start();
                    break;
            }
        }
    }

    private void stopPlaying(){
        synchronized (lock){
            requestShutdown();
            if(mConnectionType==Settings.ConnectionTypeTestFile ||mConnectionType==Settings.ConnectionTypeStorageFile){
                receiverThread.interrupt();
                //try {receiverThread.join();} catch (InterruptedException e) {e.printStackTrace();}
            }else{
                stopReceiving();
            }
            //shutdownDecoder();
            if(mConnectionType==Settings.ConnectionTypeTestFile ||mConnectionType==Settings.ConnectionTypeStorageFile){
                try {
                    receiverThread.join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            waitAndDelete();
        }
    }

    private void receiveFromAssets(){
        AssetManager assetManager=mContext.getAssets();
        InputStream in;
        byte[] buffer= new byte[BUFER_SIZE];
        try {
            in=assetManager.open(videoFromAssetsFileName);
            //in=assetManager.open("testlog.ltm");
        } catch (IOException e) {e.printStackTrace();return;}
        while(!receiverThread.isInterrupted()) {
            int sampleSize = 0;
            try {
                sampleSize=in.read(buffer,0,BUFER_SIZE);
            } catch (IOException e) {
                e.printStackTrace();}
            if(sampleSize>0){
                passNALUDataToNative(buffer,sampleSize);
            }else {
                try{in.reset();}catch (Exception e){ e.printStackTrace();}
            }
        }
        try {in.close();} catch (IOException e) {e.printStackTrace();}
        System.out.println("Receive from Assets ended");
    }

    private void receiveFromFile(String fileName) {
        java.io.FileInputStream in;
        byte[] buffer= new byte[BUFER_SIZE];
        try {
            in=new java.io.FileInputStream(Environment.getExternalStorageDirectory()+"/"+fileName);
        } catch (FileNotFoundException e) {
            System.out.println("Error opening File"+"--"+Environment.getExternalStorageDirectory()+"/"+fileName);
            return;
        }
        while(!receiverThread.isInterrupted()) {
            int sampleSize = 0;
            try {
                sampleSize=in.read(buffer,0,BUFER_SIZE);
            } catch (IOException e) {e.printStackTrace();}
            if(sampleSize>0){
                passNALUDataToNative(buffer,sampleSize);
            }else {
                try {in.close();} catch (IOException e) {e.printStackTrace();}
                try {in=new java.io.FileInputStream(Environment.getExternalStorageDirectory()+"/"+fileName);} catch (Exception e) {}
            }
        }
        System.out.println("Receive from file ended");
    }

    //called by CPP code
    //IntelliJ seems to not notice that (warning)
    //LowLagDecoder->onDecoderRatioChanged
    private void onDecoderRatioChanged(int videoW,int videoH) {
        if(mVideoParamsChangedI!=null){
            mVideoParamsChangedI.onVideoRatioChanged(videoW,videoH);
        }
    }
    //LowLagDecoder->onDecoderFpsChanged
    private void onDecoderFpsChanged(float fps) {
        if(mVideoParamsChangedI!=null){
            mVideoParamsChangedI.onVideoFPSChanged(fps);
        }
    }

    //called by CPP code NOT WORKING YET
    private void notifyUser(String text) {
        //Toaster.makeToast(mContext,text,false);
        System.out.println(text);
    }

    interface VideoParamsChanged{
        void onVideoRatioChanged(int videoW, int videoH);
        void onVideoFPSChanged(float decFPS);
    }
}
