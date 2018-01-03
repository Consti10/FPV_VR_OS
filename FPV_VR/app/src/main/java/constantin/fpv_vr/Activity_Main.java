package constantin.fpv_vr;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class Activity_Main extends AppCompatActivity implements View.OnClickListener{
    private Context mContext;

    private TestReceiver mTestReceiver=null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mContext=this;
        SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
        /*
        *  If we have not yet determined the Properties of the phone (present EGL extensions, MSAA level usw)
        *  Activity_FirstStart does exactly that. This activity may interact with the user, and returns when finished
        *  Also request all permissions needed for the app
         */
        if(pref_static.getBoolean(getString(R.string.FirstStart),true)){
            Intent i=new Intent();
            i.setClass(mContext,Activity_FirstStart.class);
            startActivity(i);
        }
        getAllPermissionsIfNotYetGranted();
        /*
         *  Update the settings. Does not block the UI thread
         */
        Settings.readFromSharedPreferencesThreaded(mContext);
    }


    @Override
    protected void onResume(){
        super.onResume();
        Settings.waitUntilSharedPreferencesAreRead();

        Button connectB=findViewById(R.id.button10);
        if(Settings.ConnectionType==Settings.ConnectionTypeTestFile){
            //statusTextView.setText("Ready to play test file");
            connectB.setTextColor(Color.GREEN);
        }else if(Settings.ConnectionType==Settings.ConnectionTypeStorageFile){
            if(MyGroundRecFileFragment.fileExists(Settings.FilenameVideo)){
                //statusTextView.setText("Ready to play ground recording file");
                connectB.setTextColor(Color.GREEN);
            }else{
                //statusTextView.setText("Cannot find ground recording file");
                connectB.setTextColor(Color.RED);
            }
        }else if(Settings.ConnectionType==Settings.ConnectionTypeManually || Settings.ConnectionType==Settings.ConnectionTypeEZWB){
            mTestReceiver=new TestReceiver(mContext,connectB);
        }
    }

    @Override
    protected void onPause(){
        super.onPause();
        if(mTestReceiver!=null){
            mTestReceiver.stopReceiving();
            mTestReceiver=null;
        }
    }

    @Override
    public void onClick(View v) {
        /*
         * Each button starts its own activity
         */
        switch (v.getId()){
            case R.id.button1:
                Intent  mVideoOnlyI=new Intent();
                mVideoOnlyI.setClass(this, Activity_MonoVid.class);
                Settings.waitUntilSharedPreferencesAreRead();
                startActivity(mVideoOnlyI);
                break;
            case R.id.button2:
                Intent mMonoI=new Intent();
                mMonoI.setClass(this, Activity_MonoVidOSD.class);
                Settings.waitUntilSharedPreferencesAreRead();
                startActivity(mMonoI);
                break;
            case R.id.button3:
                Intent mStereoI=new Intent();
                Settings.waitUntilSharedPreferencesAreRead();
                if(Settings.SynchronousFrontBufferRendering){
                    mStereoI.setClass(this, Activity_StereoSuperSYNC.class);
                    startActivity(mStereoI);
                }else{
                    mStereoI.setClass(this, Activity_Stereo.class);
                    startActivity(mStereoI);
                }
                break;
            case R.id.button5:
                Intent mSettingsI=new Intent();
                mSettingsI.setClass(this, Activity_Settings.class);
                Settings.waitUntilSharedPreferencesAreRead();
                startActivity(mSettingsI);
                break;
            case R.id.button10:
                Intent mConnectI=new Intent();
                mConnectI.setClass(this,Activity_Connect.class);
                Settings.waitUntilSharedPreferencesAreRead();
                startActivity(mConnectI);
        }
    }

    /*
    * Request all permissions needed by the app
    * TODO: For each permission. make dialog why the app needs the permission
     */
    private void getAllPermissionsIfNotYetGranted(){
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                ||ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                ||ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED
                ||ContextCompat.checkSelfPermission(this, Manifest.permission.INTERNET) != PackageManager.PERMISSION_GRANTED
                ||ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_NETWORK_STATE) != PackageManager.PERMISSION_GRANTED
                ||ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_WIFI_STATE) != PackageManager.PERMISSION_GRANTED
                ||ContextCompat.checkSelfPermission(this, Manifest.permission.CHANGE_WIFI_STATE) != PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
                            Manifest.permission.READ_EXTERNAL_STORAGE,
                            Manifest.permission.ACCESS_FINE_LOCATION,
                    Manifest.permission.CHANGE_WIFI_STATE},
                    0);
            //checking for permissions, but this doesn't block the thread
        }else {
            //they are all already granted
        }
    }

    private void makeDialog(final String message,final String title){
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setMessage(message)
                        .setTitle(title);
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }

    private boolean isFirstInstall() {
        try {
            long firstInstallTime =  getPackageManager().getPackageInfo(getPackageName(), 0).firstInstallTime;
            long lastUpdateTime = getPackageManager().getPackageInfo(getPackageName(), 0).lastUpdateTime;
            return firstInstallTime == lastUpdateTime;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            return false;
        }
    }


}
