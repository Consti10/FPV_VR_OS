package constantin.fpv_vr.AMain;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import constantin.fpv_vr.AConnect.AConnect;
import constantin.fpv_vr.Settings.ASettingsOSD;
import constantin.fpv_vr.Settings.ASettingsVR;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.APlay.AMonoVid;
import constantin.fpv_vr.APlay.AMonoVidOSD;
import constantin.fpv_vr.APlay.AStereoDaydream;
import constantin.fpv_vr.APlay.AStereoNormal;
import constantin.fpv_vr.APlay.AStereoSuperSYNC;
import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.UpdateHelper;
import constantin.renderingX.GLESInfo.AWriteGLESInfo;
import constantin.video.core.TestReceiverVideo;
import constantin.video.core.VideoNative.VideoNative;

import static constantin.fpv_vr.AConnect.AConnect.CONNECTION_TYPE_Manually;
import static constantin.fpv_vr.AConnect.AConnect.CONNECTION_TYPE_StorageFile;
import static constantin.fpv_vr.AConnect.AConnect.CONNECTION_TYPE_TestFile;
import static constantin.fpv_vr.AConnect.AConnect.CONNECTION_TYPE_EZWB;
import static constantin.fpv_vr.AConnect.AConnect.CONNECTION_TYPE_RTSP;

public class AMain extends AppCompatActivity implements View.OnClickListener{
    private static final String TAG="AMain";
    private TestReceiverVideo mTestReceiverVideo=null;
    private static final String[] REQUIRED_PERMISSION_LIST = new String[]{
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.ACCESS_FINE_LOCATION,
    };
    private final List<String> missingPermission = new ArrayList<>();
    private static final int REQUEST_PERMISSION_CODE = 12345;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        /*
         * Check ( and do the appropriate actions ) on a fresh install or update
         */
        UpdateHelper.checkForFreshInstall(this);
        AWriteGLESInfo.writeGLESInfoIfNeeded(this);
        /*
         * Same for the permissions (required in >=android X)
         */
        checkAndRequestPermissions();
    }


    @Override
    protected void onResume(){
        super.onResume();
        //Set the connectB to the right color
        Button connectB=findViewById(R.id.b_Connect);
        switch (SJ.ConnectionType(this)){
            case CONNECTION_TYPE_TestFile:
                connectB.setTextColor(Color.GREEN);
                break;
            case CONNECTION_TYPE_StorageFile:
                if(VideoNative.PLAYBACK_FLE_EXISTS(this)){
                    connectB.setTextColor(Color.GREEN);
                }else{
                    connectB.setTextColor(Color.RED);
                }
                break;
            case CONNECTION_TYPE_Manually:
            case CONNECTION_TYPE_EZWB:
                mTestReceiverVideo=new TestReceiverVideo(this,null,connectB);
                mTestReceiverVideo.startReceiving();
                break;
            case CONNECTION_TYPE_RTSP:
                connectB.setTextColor(Color.DKGRAY);
                break;
            default:
                break;
        }
    }

    @Override
    protected void onPause(){
        super.onPause();
        if(mTestReceiverVideo!=null) {
            mTestReceiverVideo.stopReceiving();
            mTestReceiverVideo=null;
        }
    }

    @Override
    public void onClick(View v) {
        /*
         * Each button starts its own activity or service
         */
        switch (v.getId()) {
            case R.id.b_startMonoVideoOnly:
                startActivity(new Intent().setClass(this, AMonoVid.class));
                break;
            case R.id.b_startMonoVideoOSD:
                startActivity(new Intent().setClass(this, AMonoVidOSD.class));
                break;
            case R.id.b_startStereo:
                Intent mStereoI = new Intent();
                //mStereoI.addCategory("com.google.intent.category.DAYDREAM");
                //mStereoI.addCategory("com.google.intent.category.CARDBOARD");
                if (SJ.DEV_USE_GVR_VIDEO_TEXTURE(this) ) {
                    mStereoI.setClass(this, AStereoDaydream.class);
                } else if (SJ.SuperSync(this)) {
                    mStereoI.setClass(this, AStereoSuperSYNC.class);
                } else {
                    mStereoI.setClass(this, AStereoNormal.class);
                }
                startActivity(mStereoI);
                break;
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


    private void checkAndRequestPermissions(){
        missingPermission.clear();
        for (String eachPermission : REQUIRED_PERMISSION_LIST) {
            if (ContextCompat.checkSelfPermission(this, eachPermission) != PackageManager.PERMISSION_GRANTED) {
                missingPermission.add(eachPermission);
            }
        }
        if (!missingPermission.isEmpty()) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                final String[] asArray=missingPermission.toArray(new String[0]);
                Log.d("PermissionManager","Request: "+Arrays.toString(asArray));
                ActivityCompat.requestPermissions(this, asArray, REQUEST_PERMISSION_CODE);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        // Check for granted permission and remove from missing list
        if (requestCode == REQUEST_PERMISSION_CODE) {
            for (int i = grantResults.length - 1; i >= 0; i--) {
                if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                    missingPermission.remove(permissions[i]);
                }
            }
        }
        if (!missingPermission.isEmpty()) {
            checkAndRequestPermissions();
        }

    }






}
