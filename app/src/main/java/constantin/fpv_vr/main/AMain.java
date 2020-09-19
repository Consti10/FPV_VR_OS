package constantin.fpv_vr.main;

import android.Manifest;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.media.projection.MediaProjectionManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.hbisoft.hbrecorder.HBRecorder;
import com.hbisoft.hbrecorder.HBRecorderListener;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import constantin.fpv_vr.R;
import constantin.fpv_vr.OSD2.ATestlayout;
import constantin.fpv_vr.Toaster;
import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.fpv_vr.play_mono.AMonoVideoOSD;
import constantin.fpv_vr.play_stereo.AStereoVR;
import constantin.fpv_vr.settings.AGroundRecordingSettings;
import constantin.fpv_vr.settingsOSD.ASettingsOSD;
import constantin.fpv_vr.settings.SJ;
import constantin.fpv_vr.settings.UpdateHelper;
import constantin.renderingx.core.gles_info.AWriteGLESInfo;
import constantin.test.UVCHelper;
import constantin.video.core.RequestPermissionHelper;
import constantin.video.core.TestReceiverVideo;
import constantin.video.core.player.VideoSettings;

import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_EZWB;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_Manually;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_RTSP;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_StorageFile;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_TestFile;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_UVC;

import constantin.fpv_vr.databinding.ActivityMainBinding;

public class AMain extends AppCompatActivity implements View.OnClickListener , HBRecorderListener {
    private static final String TAG=AMain.class.getSimpleName();
    private TestReceiverVideo mTestReceiverVideo=null;
    private final RequestPermissionHelper requestPermissionHelper=new RequestPermissionHelper(new String[]{
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.ACCESS_FINE_LOCATION,
                    Manifest.permission.CAMERA
            }
    );
    private HBRecorder hbRecorder;
    private static final int SCREEN_RECORD_REQUEST_CODE = 777;
    //If this Intent != null the permission to record screen was already granted
    Intent mRecordScreenPermissionI=null;
    int mRecordScreenResultCode;
    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
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
        notifyUserStartedForUVC();
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
        switch (SJ.getConnectionType(this)){
            case CONNECTION_TYPE_TestFile:
                binding.bConnect.setTextColor(Color.GREEN);
                break;
            case CONNECTION_TYPE_StorageFile:
                if(VideoSettings.PLAYBACK_FLE_EXISTS(this)){
                    binding.bConnect.setTextColor(Color.GREEN);
                }else{
                    binding.bConnect.setTextColor(Color.RED);
                }
                break;
            case CONNECTION_TYPE_Manually:
            case CONNECTION_TYPE_EZWB:
                if(mTestReceiverVideo==null){
                    mTestReceiverVideo=new TestReceiverVideo(this);
                }
                mTestReceiverVideo.setViews(null,binding.bConnect);
                break;
            case CONNECTION_TYPE_RTSP:
                binding.bConnect.setTextColor(Color.DKGRAY);
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
        if(v==binding.bStartMonoVideoOnly || v==binding.bStartMonoVideoOSD){
            if(DJIApplication.isDJIEnabled(this) && !DJIApplication.isAircraftConnected()){
                Toaster.makeToast(this,"No connected product",false);
                return;
            }
            final Intent intent=new Intent().setClass(this, AMonoVideoOSD.class);
            intent.putExtra(AMonoVideoOSD.EXTRA_KEY_ENABLE_OSD,v.getId()==R.id.b_startMonoVideoOSD);
            startActivity(intent);
            startRecordingScreenIfEnabled();
        }else if(v==binding.bStartStereo){
            if(DJIApplication.isDJIEnabled(this) && !DJIApplication.isAircraftConnected()){
                Toaster.makeToast(this,"No connected product",false);
                return;
            }
            final Intent intent = new Intent();
            intent.setClass(this, AStereoVR.class);
            startActivity(intent);
            startRecordingScreenIfEnabled();
        }else if(v==binding.bOSDSettings){
            startActivity(new Intent().setClass(this, ASettingsOSD.class));
        }else if(v==binding.bConnect){
            startActivity(new Intent().setClass(this, AConnect.class));
        }else if(v==binding.bExp){
            startActivity(new Intent().setClass(this, ATestlayout.class));
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
                setOutputPath();
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
    public void HBRecorderOnStart() {

    }

    //For Android 10> we will pass a Uri to HBRecorder
    //This is not necessary - You can still use getExternalStoragePublicDirectory
    //But then you will have to add android:requestLegacyExternalStorage="true" in your Manifest
    //IT IS IMPORTANT TO SET THE FILE NAME THE SAME AS THE NAME YOU USE FOR TITLE AND DISPLAY_NAME
    ContentResolver resolver;
    ContentValues contentValues;
    Uri mUri;
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private void setOutputPath() {
        String filename = generateFileName();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            resolver = getContentResolver();
            contentValues = new ContentValues();
            contentValues.put(MediaStore.Video.Media.RELATIVE_PATH, "Movies/" + "FPV_VR");
            contentValues.put(MediaStore.Video.Media.TITLE, filename);
            contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, filename);
            contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "video/mp4");
            mUri = resolver.insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, contentValues);
            //FILE NAME SHOULD BE THE SAME
            hbRecorder.setFileName(filename);
            hbRecorder.setOutputUri(mUri);
        }else{
            createFolder();
            hbRecorder.setOutputPath(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES) +"/HBRecorder");
        }
    }
    //Generate a timestamp to be used as a file name
    private String generateFileName() {
        SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss", Locale.getDefault());
        Date curDate = new Date(System.currentTimeMillis());
        return formatter.format(curDate).replace(" ", "");
    }
    //Create Folder
    //Only call this on Android 9 and lower (getExternalStoragePublicDirectory is deprecated)
    //This can still be used on Android 10> but you will have to add android:requestLegacyExternalStorage="true" in your Manifest
    private void createFolder() {
        File f1 = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES), "HBRecorder");
        if (!f1.exists()) {
            if (f1.mkdirs()) {
                Log.i("Folder ", "created");
            }
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
    // Notify user if mode is probably UVC
    private void notifyUserStartedForUVC(){
        if (UVCHelper.startedViaIntentFilterActionUSB(this)) {
            if(SJ.getConnectionType(this)!=CONNECTION_TYPE_UVC){
                final String message="Select UVC as connection type";
                new AlertDialog.Builder(this).setMessage(message).setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        startActivity(new Intent().setClass(AMain.this, AConnect.class));
                    }
                }).setNegativeButton("No",null).show();
            }
        }
    }
}
