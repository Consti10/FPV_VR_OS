package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.fragment.app.FragmentManager;

import com.google.android.material.snackbar.Snackbar;

import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.AGroundRecordingSettings;
import constantin.fpv_vr.Settings.SJ;
import constantin.telemetry.core.ASettingsTelemetry;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.AVideoSettings;
import constantin.video.core.VideoPlayer.VideoPlayer;
import constantin.video.core.VideoPlayer.VideoSettings;

/************************************
 * This activity only writes values in pref_connect.xml .
 * 4 distinct Fragments (user can change between them via toolbar spinner) for connecting to the receiver
 * 1)EZ-Wifibroadcast: (FConnectWB)
 *  - DATA_SOURCE UDP
 *  - Connect forwards the user to Wifi or tethering settings
 *  - when a usb/wifi connection is detected test receiver is opened
 * 2) MANUALLY (FConnectManually)
 *  - DATA_SOURCE UDP
 *  - Immediately opens test receiver on the specified Video/TelemetryReceiver ports
 * 3) TestFile (FConnectTestFile)
 *  - DATA_SOURCE ASSETS
 *  - shows the first frame of the test video as picture
 * 4) Ground Recorded file (FConnectGroundRecFile)
 *  - DATA_SOURCE File
 *  - filePointer: selected file
 *  - user can select a ground recording file
 ************************************/


@SuppressLint("ApplySharedPref")
public class AConnect extends AppCompatActivity implements AdapterView.OnItemSelectedListener{
    private Context mContext;

    public static final int CONNECTION_TYPE_EZWB =0;
    public static final int CONNECTION_TYPE_Manually =1;
    public static final int CONNECTION_TYPE_TestFile =2;
    public static final int CONNECTION_TYPE_StorageFile =3;
    public static final int CONNECTION_TYPE_RTSP =4;
    public static final int CONNECTION_TYPE_DJI =5;

    @Override
    public boolean onCreateOptionsMenu(Menu menu){
        MenuInflater inflater=getMenuInflater();
        inflater.inflate(R.menu.main_toolbar_menu,menu);
        return true;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connect);
        Toolbar myToolbar =  findViewById(R.id.connect_toolbar);
        myToolbar.setTitleTextColor(Color.WHITE);
        setSupportActionBar(myToolbar);
        getSupportActionBar().setTitle("");
        mContext=this;
    }

    @Override
    protected void onResume(){
        super.onResume();
        Spinner mSpinner=findViewById(R.id.spinner_connection_type);
        mSpinner.setOnItemSelectedListener(this);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,R.array.entriesConnectionType,R.layout.spinner_white);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(adapter);
        mSpinner.setSelection(SJ.getConnectionType(this));
    }

    @Override
    protected void onPause(){
        super.onPause();
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_settings_telemetry:
                startActivity(new Intent().setClass(this, ASettingsTelemetry.class));
                return true;
            case R.id.action_settings_video:
                startActivity(new Intent().setClass(this, AVideoSettings.class));
                return true;
            case R.id.action_ground_recording:
                startActivity(new Intent().setClass(this, AGroundRecordingSettings.class));
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        //The toolbar has changed. Replace the old fragment with a new one
        final FragmentManager fm=getSupportFragmentManager();
        final View v=findViewById(R.id.spinner_connection_type);
        setPreferencesForConnectionType(this,position);
        switch (position){
            case CONNECTION_TYPE_EZWB: //EZ-Wifibroadcast (Network)
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectWB()).commit();
                makeSnackBarForView(mContext,"connection type set to EZWB (UDP)",v);
                break;
            case CONNECTION_TYPE_Manually: //Manually (Network)
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectManually()).commit();
                makeSnackBarForView(mContext,"connection type set to Manually(UDP)",v);
                break;
            case CONNECTION_TYPE_TestFile: //Assets file
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectTestFile()).commit();
                makeSnackBarForView(mContext,"connection type set to Test file (Assets)",v);
                break;
            case CONNECTION_TYPE_StorageFile://storage file
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectGroundRecFile()).commit();
                makeSnackBarForView(mContext,"connection type set to GRecFile (File)",v);
                break;
            case CONNECTION_TYPE_RTSP: //RTSP
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectRTSP()).commit();
                makeSnackBarForView(mContext,"connection type set to RTSP",v);
                break;
            case CONNECTION_TYPE_DJI:
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectDJI()).commit();
                makeSnackBarForView(mContext,"connection type set to DJI",v);
                break;
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
    }

    private static void makeSnackBarForView(final Context activityContext, final String message, final View v){
        ((Activity)activityContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final Snackbar snackbar;
                if(v!=null){
                    snackbar=Snackbar.make(v, message, Snackbar.LENGTH_INDEFINITE);
                    snackbar.setAction("OK", new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            snackbar.dismiss();
                        }
                    });
                    snackbar.show();
                }
            }
        });
    }

    public static void setPreferencesForConnectionType(final Context context,final int connectionType){
        switch (connectionType){
            case CONNECTION_TYPE_EZWB:
            case CONNECTION_TYPE_Manually:
                VideoSettings.setVS_SOURCE(context, VideoSettings.VS_SOURCE.UDP);
                TelemetrySettings.setT_SOURCE(context,TelemetrySettings.SOURCE_TYPE_UDP);
                break;
            case CONNECTION_TYPE_StorageFile:
                VideoSettings.setVS_SOURCE(context,VideoSettings.VS_SOURCE.FILE);
                TelemetrySettings.setT_SOURCE(context,TelemetrySettings.SOURCE_TYPE_FILE);
                break;
            case CONNECTION_TYPE_TestFile:
                VideoSettings.setVS_SOURCE(context,VideoSettings.VS_SOURCE.ASSETS);
                final int vm= VideoSettings.videoMode(context);
                if(vm==0){
                    VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(context, "x264/testVideo.h264");
                }else if(vm==1){
                    VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(context,  "360/insta_webbn_1_shortened.h264");
                }else{
                    VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(context,  "360/insta_webbn_1_shortened.h264");
                }
                //pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "outfile.h264");
                //pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "video360.h264");
                //pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "paris_by_diego.h264");
                //pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "mono.h264");
                //pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "testVideo.h264");
                TelemetrySettings.setT_SOURCE(context,TelemetrySettings.SOURCE_TYPE_ASSETS);
                break;
            case CONNECTION_TYPE_RTSP:
                VideoSettings.setVS_SOURCE(context, VideoSettings.VS_SOURCE.FFMPEG);
                TelemetrySettings.setT_SOURCE(context,TelemetrySettings.SOURCE_TYPE_UDP);
                break;
            case CONNECTION_TYPE_DJI:
                VideoSettings.setVS_SOURCE(context, VideoSettings.VS_SOURCE.EXTERNAL);
                TelemetrySettings.setT_SOURCE(context,TelemetrySettings.SOURCE_TYPE_EXTERNAL_DJI);
                break;
             default:break;
        }
        SJ.setConnectionType(context,connectionType);
    }


}
