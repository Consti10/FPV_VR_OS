package constantin.fpv_vr.main;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.media.projection.MediaProjectionManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.hbisoft.hbrecorder.HBRecorder;
import com.hbisoft.hbrecorder.HBRecorderListener;

import constantin.fpv_vr.R;
import constantin.fpv_vr.Toaster;
import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.play_mono.AMonoVideoOSD;
import constantin.fpv_vr.play_stereo.AStereoNormal;
import constantin.fpv_vr.play_stereo.AStereoSuperSYNC;
import constantin.fpv_vr.settings.AGroundRecordingSettings;
import constantin.fpv_vr.settings.ASettingsOSD;
import constantin.fpv_vr.settings.ASettingsVR;
import constantin.fpv_vr.settings.SJ;
import constantin.fpv_vr.settings.UpdateHelper;
import constantin.fpv_vr.djiintegration.xdji.DJIApplication;
import constantin.fpv_vr.xexperimental.AStereoDaydream;
import constantin.renderingx.core.gles_info.AWriteGLESInfo;
import constantin.video.core.RequestPermissionHelper;
import constantin.video.core.TestReceiverVideo;
import constantin.video.core.video_player.VideoSettings;

import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_EZWB;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_Manually;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_RTSP;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_StorageFile;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_TestFile;

public class AMain extends AppCompatActivity implements View.OnClickListener , HBRecorderListener {
    private static final String TAG="AMain";
    private TestReceiverVideo mTestReceiverVideo=null;
    private final RequestPermissionHelper requestPermissionHelper=new RequestPermissionHelper(new String[]{
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.ACCESS_FINE_LOCATION,
            }
    );
    //
    private HBRecorder hbRecorder;
    private static final int SCREEN_RECORD_REQUEST_CODE = 777;
    //If this Intent != null the permission to record screen was already granted
    Intent mRecordScreenPermissionI=null;
    int mRecordScreenResultCode;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        /*
         * Check ( and do the appropriate actions ) on a fresh install or update
         */
        UpdateHelper.checkForFreshInstallOrUpdate(this);
        AWriteGLESInfo.writeGLESInfoIfNeeded(this);
        /*
         * Same for the permissions (required in >=android X)
         */
        requestPermissionHelper.checkAndRequestPermissions(this);
        //if(!DJISDKManager.getInstance().hasSDKRegistered()){
        //    startActivity(new Intent().setClass(this, DJIConnectionA.class));
        //}
    }

    @Override
    protected void onResume(){
        super.onResume();
        //Get the permission to record screen if needed
        if(AGroundRecordingSettings.enableHBScreenRecorder(this)){
            getPermissionRecordScreenIfNeeded();
        }
        //If a screen recording is running it means we came back to AMain from a playing activity
        stopRecordingScreenIfNeeded();
        //Set the connectB to the right color
        Button connectB=findViewById(R.id.b_Connect);
        switch (SJ.getConnectionType(this)){
            case CONNECTION_TYPE_TestFile:
                connectB.setTextColor(Color.GREEN);
                break;
            case CONNECTION_TYPE_StorageFile:
                if(VideoSettings.PLAYBACK_FLE_EXISTS(this)){
                    connectB.setTextColor(Color.GREEN);
                }else{
                    connectB.setTextColor(Color.RED);
                }
                break;
            case CONNECTION_TYPE_Manually:
            case CONNECTION_TYPE_EZWB:
                if(mTestReceiverVideo==null){
                    mTestReceiverVideo=new TestReceiverVideo(this);
                }
                mTestReceiverVideo.setViews(null,connectB);
                break;
            case CONNECTION_TYPE_RTSP:
                connectB.setTextColor(Color.DKGRAY);
                break;
            default:
                break;
        }
    }

    @Override
    public void onClick(View v) {
        /*
         * Each button starts its own activity or service
         */
        switch (v.getId()) {
            case R.id.b_startMonoVideoOnly:
            case R.id.b_startMonoVideoOSD:{
                if(DJIApplication.isDJIEnabled(this) && !DJIApplication.isAircraftConnected()){
                    Toaster.makeToast(this,"No connected product",false);
                    return;
                }
                final Intent intent=new Intent().setClass(this, AMonoVideoOSD.class);
                intent.putExtra(AMonoVideoOSD.EXTRA_KEY_ENABLE_OSD,v.getId()==R.id.b_startMonoVideoOSD);
                startActivity(intent);
                startRecordingScreenIfEnabled();
            }break;
            case R.id.b_startStereo:{
                if(DJIApplication.isDJIEnabled(this) && !DJIApplication.isAircraftConnected()){
                    Toaster.makeToast(this,"No connected product",false);
                    return;
                }
                final Intent intent = new Intent();
                //mStereoI.addCategory("com.google.intent.category.DAYDREAM");
                //mStereoI.addCategory("com.google.intent.category.CARDBOARD");
                if (SJ.DEV_USE_GVR_VIDEO_TEXTURE(this)) {
                    intent.setClass(this, AStereoDaydream.class);
                } else if (SJ.SuperSync(this)) {
                    intent.setClass(this, AStereoSuperSYNC.class);
                } else {
                    intent.setClass(this, AStereoNormal.class);
                }
                startActivity(intent);
                startRecordingScreenIfEnabled();
                break;
            }
            case R.id.b_OSDSettings:
                startActivity(new Intent().setClass(this, ASettingsOSD.class));
                break;
            case R.id.b_Connect:
                startActivity(new Intent().setClass(this, AConnect.class));
                break;
            case R.id.b_VRSettings:
                startActivity(new Intent().setClass(this, ASettingsVR.class));
                break;
        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        requestPermissionHelper.onRequestPermissionsResult(requestCode,permissions,grantResults);
    }

    //After the permission was granted by the user,
    //the Intent gets set and stored for later use via the onActivityResult callback
    private void getPermissionRecordScreenIfNeeded(){
        if(mRecordScreenPermissionI!=null)return;
        MediaProjectionManager mediaProjectionManager = (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        Intent permissionIntent = mediaProjectionManager != null ? mediaProjectionManager.createScreenCaptureIntent() : null;
        startActivityForResult(permissionIntent, SCREEN_RECORD_REQUEST_CODE);
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == SCREEN_RECORD_REQUEST_CODE) {
            if (resultCode == RESULT_OK) {
                mRecordScreenPermissionI =data;
                mRecordScreenResultCode =resultCode;
            }
        }
    }

    private void startRecordingScreenIfEnabled(){
        if(AGroundRecordingSettings.enableHBScreenRecorder(this)){
            if(mRecordScreenPermissionI==null){
                Toaster.makeToast(this,"Cannot enable mp4 ground recording",false);
                return;
            }
            if(hbRecorder==null){
                hbRecorder = new HBRecorder(this, this);
                hbRecorder.setVideoFrameRate(AGroundRecordingSettings.getGROUND_RECORDING_VIDEO_FPS(this));
                hbRecorder.setVideoBitrate(AGroundRecordingSettings.getGROUND_RECORDING_VIDEO_BITRATE(this));
                hbRecorder.isAudioEnabled(false);
            }
            hbRecorder.startScreenRecording(mRecordScreenPermissionI, mRecordScreenResultCode, this);
            System.out.println("hbRecorder Start screen recorder");
        }
    }

    private void stopRecordingScreenIfNeeded(){
        if(hbRecorder!=null && hbRecorder.isBusyRecording()){
            hbRecorder.stopScreenRecording();
            System.out.println("hbRecorder Stop screen recorder");
        }
    }

    @Override
    public void HBRecorderOnComplete() {
        System.out.println("HBRecorderOnComplete()");
    }

    @Override
    public void HBRecorderOnError(int errorCode, String reason) {
        System.out.println("HBRecorderOnError "+errorCode+" "+reason);
    }
}
