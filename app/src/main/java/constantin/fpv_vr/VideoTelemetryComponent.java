package constantin.fpv_vr;

import android.view.GestureDetector;
import android.view.SurfaceHolder;
import androidx.appcompat.app.AppCompatActivity;

import constantin.fpv_vr.connect.AConnect;
//XDJIimport constantin.fpv_vr.djiintegration.TelemetryReceiverDJI;
//XDJIimport constantin.fpv_vr.djiintegration.VideoPlayerDJI;
import constantin.fpv_vr.settings.SJ;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.uvcintegration.UVCPlayer;
import constantin.video.core.player.IVideoParamsChanged;
import constantin.video.core.gl.ISurfaceTextureAvailable;
import constantin.video.core.player.VideoPlayer;

/**
 * Depending on the connection type selected by the user this component
 * Instantiates the proper video and telemetry receiver while also providing a convenient
 * abstraction for configuring all the different player(s)
 */
public class VideoTelemetryComponent extends GestureDetector.SimpleOnGestureListener {
    private static final String TAG=VideoTelemetryComponent.class.getSimpleName();
    private final int connectionType;
    private VideoPlayer videoPlayer;
    //XDJIprivate VideoPlayerDJI videoPlayerDJI;
    private UVCPlayer uvcPlayer;
    private TelemetryReceiver telemetryReceiver;
    //XDJIprivate TelemetryReceiverDJI telemetryReceiverDJI;

    public VideoTelemetryComponent(final AppCompatActivity parent){
        connectionType=SJ.getConnectionType(parent);
        if(connectionType== AConnect.CONNECTION_TYPE_UVC){
            uvcPlayer=new UVCPlayer(parent);
            telemetryReceiver=new TelemetryReceiver(parent,0,0,0);
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            //XDJIvideoPlayerDJI=new VideoPlayerDJI(parent);
            //XDJItelemetryReceiverDJI=new TelemetryReceiverDJI(parent,0,0);
        }else{
            videoPlayer=new VideoPlayer(parent);
            telemetryReceiver=new TelemetryReceiver(parent,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer(),0);
        }
    }

    public TelemetryReceiver getTelemetryReceiver(){
        if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            //XDJIreturn telemetryReceiverDJI;
            return null;
        }
        return telemetryReceiver;
    }

    public SurfaceHolder.Callback configure1(){
        if(connectionType==AConnect.CONNECTION_TYPE_UVC){
            return uvcPlayer.configure1();
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            //XDJIreturn videoPlayerDJI.configure1();
            return null;
        }else{
            return videoPlayer.configure1();
        }
    }

    public ISurfaceTextureAvailable configure2(){
        if(connectionType==AConnect.CONNECTION_TYPE_UVC){
            return uvcPlayer.configure2();
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            //XDJIreturn videoPlayerDJI.configure2();
            return null;
        }else{
            return videoPlayer.configure2();
        }
    }

    public void setIVideoParamsChanged(final IVideoParamsChanged iVideoParamsChanged){
        if(connectionType==AConnect.CONNECTION_TYPE_UVC){
            uvcPlayer.setIVideoParamsChanged(iVideoParamsChanged);
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            //XDJIvideoPlayerDJI.setIVideoParamsChanged(iVideoParamsChanged);
        }else{
            videoPlayer.setIVideoParamsChanged(iVideoParamsChanged);
        }
    }


}
