package constantin.fpv_vr.connect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
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
import constantin.fpv_vr.settings.AGroundRecordingSettings;
import constantin.fpv_vr.settings.SJ;
import constantin.telemetry.core.ASettingsTelemetry;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.AVideoSettings;
import constantin.video.core.player.VideoSettings;

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
    public static final int CONNECTION_TYPE_UVC=6;

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
        myToolbar.setTitle("");
        setSupportActionBar(myToolbar);
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
        final int id=item.getItemId();
        if(id==R.id.action_settings_telemetry){
            startActivity(new Intent().setClass(this, ASettingsTelemetry.class));
            return true;
        }else if(id==R.id.action_settings_video){
            startActivity(new Intent().setClass(this, AVideoSettings.class));
            return true;
        }else if(id==R.id.action_ground_recording){
            startActivity(new Intent().setClass(this, AGroundRecordingSettings.class));
            return true;
        }else{
            return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        //The toolbar has changed. Replace the old fragment with a new one
        final FragmentManager fm=getSupportFragmentManager();
        final View v=findViewById(R.id.spinner_connection_type);
        setPreferencesForConnectionType(this,position);
        // https://stackoverflow.com/questions/7575921/illegalstateexception-can-not-perform-this-action-after-onsaveinstancestate-wit
        // i just changed it to commitNow and hope that fixes it
        switch (position){
            case CONNECTION_TYPE_EZWB: //EZ-Wifibroadcast (Network)
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectWB()).commitNow();
                makeSnackBarForView(mContext,"connection type set to EZWB (UDP)",v);
                break;
            case CONNECTION_TYPE_Manually: //Manually (Network)
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectManually()).commitNow();
                makeSnackBarForView(mContext,"connection type set to Manually(UDP)",v);
                break;
            case CONNECTION_TYPE_TestFile: //Assets file
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectTestFile()).commitNow();
                makeSnackBarForView(mContext,"connection type set to Test file (Assets)",v);
                break;
            case CONNECTION_TYPE_StorageFile://storage file
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectGroundRecFile()).commitNow();
                makeSnackBarForView(mContext,"connection type set to GRecFile (File)",v);
                break;
            case CONNECTION_TYPE_RTSP: //RTSP
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectRTSP()).commitNow();
                makeSnackBarForView(mContext,"connection type set to RTSP",v);
                break;
            case CONNECTION_TYPE_DJI:
                // Will crash on emulator !
                //XDJIfm.beginTransaction().replace(R.id.fragment_container, new FConnectDJI()).commitNow();
                //XDJImakeSnackBarForView(mContext,"connection type set to DJI",v);
                break;
            case CONNECTION_TYPE_UVC:
                fm.beginTransaction().replace(R.id.fragment_container, new FConnectUVC()).commitNow();
                makeSnackBarForView(mContext,"connection type set to UVC",v);
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
                VideoSettings.setVS_ASSETS_FILENAME_TEST_ONLY(context, "fpv/zipray_shortened.fpv");
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
            case CONNECTION_TYPE_UVC:
                VideoSettings.setVS_SOURCE(context, VideoSettings.VS_SOURCE.EXTERNAL);
                TelemetrySettings.setT_SOURCE(context,TelemetrySettings.SOURCE_TYPE_UDP);
                break;
             default:break;
        }
        SJ.setConnectionType(context,connectionType);
    }


}
