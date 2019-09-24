package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import com.google.android.material.snackbar.Snackbar;

import androidx.appcompat.widget.Toolbar;
import androidx.fragment.app.FragmentManager;
import androidx.appcompat.app.AppCompatActivity;

import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import constantin.fpv_vr.AMain.AMain;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.R;
import constantin.telemetry.core.ASettingsTelemetry;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.video.core.AVideoSettings;
import constantin.video.core.VideoNative.VideoNative;

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
        mSpinner.setSelection(SJ.ConnectionType(this));
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
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        //The toolbar has changed. Replace the old fragment with a new one
        //FragmentManager fm=getFragmentManager();
        FragmentManager fm=getSupportFragmentManager();
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
        SharedPreferences pref_video=context.getSharedPreferences("pref_video",Context.MODE_PRIVATE);
        SharedPreferences pref_telemetry=context.getSharedPreferences("pref_telemetry",MODE_PRIVATE);
        SharedPreferences.Editor pref_video_edit=pref_video.edit();
        SharedPreferences.Editor pref_telemetry_edit=pref_telemetry.edit();
        switch (connectionType){
            case CONNECTION_TYPE_EZWB:
            case CONNECTION_TYPE_Manually:
                pref_video_edit.putInt(context.getString(R.string.VS_SOURCE), VideoNative.VS_SOURCE_UDP);
                pref_telemetry_edit.putInt(context.getString(R.string.T_SOURCE), TelemetryReceiver.SOURCE_TYPE_UDP);
                break;
            case CONNECTION_TYPE_StorageFile:
                pref_video_edit.putInt(context.getString(R.string.VS_SOURCE),VideoNative.VS_SOURCE_FILE);
                pref_telemetry_edit.putInt(context.getString(R.string.T_SOURCE),TelemetryReceiver.SOURCE_TYPE_FILE);
                break;
            case CONNECTION_TYPE_TestFile:
                pref_video_edit.putInt(context.getString(R.string.VS_SOURCE),VideoNative.VS_SOURCE_ASSETS);
                if(VideoNative.video360(context)){
                    //pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "outfile.h264");
                    pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "video360.h264");
                    //pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "mono.h264");
                }else{
                    pref_video_edit.putString(context.getString(R.string.VS_ASSETS_FILENAME_TEST_ONLY), "testVideo.h264");
                }
                pref_telemetry_edit.putInt(context.getString(R.string.T_SOURCE),TelemetryReceiver.SOURCE_TYPE_ASSETS);
                break;
            case CONNECTION_TYPE_RTSP:
                pref_video_edit.putInt(context.getString(R.string.VS_SOURCE),VideoNative.VS_SOURCE_FFMPEG_URL);
                pref_telemetry_edit.putInt(context.getString(R.string.T_SOURCE),TelemetryReceiver.SOURCE_TYPE_UDP);
                break;
             default:break;
        }
        pref_video_edit.commit();
        pref_telemetry_edit.commit();
        context.getSharedPreferences("pref_connect", MODE_PRIVATE).edit().
                putInt(context.getString(R.string.ConnectionType), connectionType).commit();
    }


}
