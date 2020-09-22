package constantin.fpv_vr;

import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.appcompat.app.AppCompatActivity;

import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.djiintegration.TelemetryReceiverDJI;
import constantin.fpv_vr.djiintegration.VideoPlayerDJI;
import constantin.fpv_vr.settings.SJ;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.uvcintegration.UVCPlayer;
import constantin.video.core.IVideoParamsChanged;
import constantin.video.core.gl.ISurfaceTextureAvailable;
import constantin.video.core.player.VideoPlayer;

/**
 * Depending on the connection type selected by the user this component
 * Instantiates the proper video and telemetry receiver while also providing a convenient
 * abstraction for configuring all the different player(s)
 */
public class VideoTelemetryComponent {
    private final int connectionType;
    private VideoPlayer videoPlayer;
    private VideoPlayerDJI videoPlayerDJI;
    private UVCPlayer uvcPlayer;
    private TelemetryReceiver telemetryReceiver;
    private TelemetryReceiverDJI telemetryReceiverDJI;

    public VideoTelemetryComponent(final AppCompatActivity parent){
        connectionType=SJ.getConnectionType(parent);
        if(connectionType== AConnect.CONNECTION_TYPE_UVC){
            uvcPlayer=new UVCPlayer(parent);
            telemetryReceiver=new TelemetryReceiver(parent,0,0);
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            videoPlayerDJI=new VideoPlayerDJI(parent);
            telemetryReceiverDJI=new TelemetryReceiverDJI(parent,0,0);
        }else{
            videoPlayer=new VideoPlayer(parent);
            telemetryReceiver=new TelemetryReceiver(parent,videoPlayer.getExternalGroundRecorder(),videoPlayer.getExternalFilePlayer());
        }
    }

    public TelemetryReceiver getTelemetryReceiver(){
        if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            return telemetryReceiverDJI;
        }
        return telemetryReceiver;
    }

    public SurfaceHolder.Callback configure1(){
        if(connectionType==AConnect.CONNECTION_TYPE_UVC){
            return uvcPlayer.configure1();
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            return videoPlayerDJI.configure1();
        }else{
            return videoPlayer.configure1();
        }
    }

    public ISurfaceTextureAvailable configure2(){
        if(connectionType==AConnect.CONNECTION_TYPE_UVC){
            return uvcPlayer.configure2();
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
            return videoPlayerDJI.configure2();
        }else{
            return videoPlayer.configure2();
        }
    }

    public void setIVideoParamsChanged(final IVideoParamsChanged iVideoParamsChanged){
        if(connectionType==AConnect.CONNECTION_TYPE_UVC){
            uvcPlayer.setIVideoParamsChanged(iVideoParamsChanged);
        }else if(connectionType==AConnect.CONNECTION_TYPE_DJI){
           videoPlayerDJI.setIVideoParamsChanged(iVideoParamsChanged);
        }else{
            videoPlayer.setIVideoParamsChanged(iVideoParamsChanged);
        }
    }
}
