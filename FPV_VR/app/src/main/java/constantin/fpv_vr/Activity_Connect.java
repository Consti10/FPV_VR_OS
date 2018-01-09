package constantin.fpv_vr;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.FragmentManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Spinner;

/************************************
 * This activity only writes values in pref_connect.xml .
 * 4 distinct Fragments (user can change between them via toolbar spinner) for connecting to the receiver
 * 1)EZ-Wifibroadcast: (MyEZWBFragment)
 *  - DATA_SOURCE UDP
 *  - Connect forwards the user to Wifi or tethering settings
 *  - when a usb/wifi connection is detected test receiver is opened
 * 2) MANUALLY (MyManuallyFragment)
 *  - DATA_SOURCE UDP
 *  - Immediately opens test receiver on the specified Video/Telemetry ports
 * 3) TestFile (MyTestFileFragment)
 *  - DATA_SOURCE ASSETS
 *  - shows the first frame of the test video as picture
 * 4) Ground Recorded file (MyGroundRecFileFragment)
 *  - DATA_SOURCE File
 *  - filePointer: selected file
 *  - user can select a ground recording file
 ************************************/

@SuppressLint("ApplySharedPref")
public class Activity_Connect extends FragmentActivity implements AdapterView.OnItemSelectedListener,View.OnClickListener{

    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connect);
        mContext=this;
    }

    @Override
    protected void onResume(){
        super.onResume();
        Spinner mSpinner=findViewById(R.id.spinner_nav);
        mSpinner.setOnItemSelectedListener(this);
        SharedPreferences pref_connect = getSharedPreferences("pref_connect", MODE_PRIVATE);
        String connectionType=pref_connect.getString(mContext.getString(R.string.ConnectionType),""+Settings.ConnectionTypeTestFile);
        mSpinner.setSelection(Integer.parseInt(connectionType));
    }

    @Override
    protected void onPause(){
        super.onPause();
        Settings.readFromSharedPreferencesThreaded(mContext);
    }

    @Override
    public void onClick(View v) {
        //No matter which fragment is currently attached: all the  clicks on buttons usw (except the Spinner in the Top Bar) that have one of these actions:
        //1)info dialogue
        //2)connect via wifi or usb action
        //are handled in this callback
        switch (v.getId()){
            case R.id.Mode1InfoB:
                makeInfoDialog(mContext.getString(R.string.EZWBMode1Info));
                break;
            case R.id.Mode2InfoB:
                makeInfoDialog(mContext.getString(R.string.EZWBMode2Info));
                break;
            case R.id.ManuallyInfoB:
                makeInfoDialog(mContext.getString(R.string.ManuallyInfo));
                break;
            case R.id.Mode1ConnectB:
                Toaster.makeToast(mContext,"Connect to SSID=EZ-WifiBroadcast\nPASSWORD=wifibraodcast",true);
                startActivity(new Intent(android.provider.Settings.ACTION_WIFI_SETTINGS));
                break;
            case R.id.Mode2ConnectB:
                if(!IsConnected.isUSBConnected(mContext)){
                    makeInfoDialog("Please check your usb connection");
                    return;
                }
                final Intent intent = new Intent(Intent.ACTION_MAIN, null);
                intent.addCategory(Intent.CATEGORY_LAUNCHER);
                final ComponentName cn = new ComponentName("com.android.settings", "com.android.settings.TetherSettings");
                intent.setComponent(cn);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                Toaster.makeToast(mContext,"enable 'USB tethering' (not wifi,but usb hotspot)",true);
                startActivity( intent);
                break;
            case R.id.network_settings:
                //getFragmentManager().beginTransaction().add(android.R.id.content,new NetworkSettingsFrag()).commit();
                //getFragmentManager().beginTransaction().add(R.id.fragment_container,new NetworkSettingsFrag()).commit();
                Intent mConnectI=new Intent();
                mConnectI.setClass(mContext,Activity_ConnectSub.class);
                startActivity(mConnectI);
                break;
            default:
                break;
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        //The toolbar has changed. Replace the old fragment with a new one
        FragmentManager fm=getFragmentManager();
        SharedPreferences.Editor mPrefConnectEdit=getSharedPreferences("pref_connect", MODE_PRIVATE).edit();
        switch (position){
            case Settings.ConnectionTypeEZWB: //EZ-Wifibroadcast (Network)
                mPrefConnectEdit.putString(mContext.getString(R.string.ConnectionType),""+Settings.ConnectionTypeEZWB).commit();
                Settings.ConnectionType =Settings.ConnectionTypeEZWB;
                fm.beginTransaction().replace(R.id.fragment_container, new MyEZWBFragment()).commit();
                Snacker.makeSnackBarForSpinner(mContext,"connection type set to EZWB (UDP)");
                break;
            case Settings.ConnectionTypeManually: //Manually (Network)
                mPrefConnectEdit.putString(mContext.getString(R.string.ConnectionType),""+Settings.ConnectionTypeManually).commit();
                Settings.ConnectionType =Settings.ConnectionTypeManually;
                fm.beginTransaction().replace(R.id.fragment_container, new MyManuallyFragment()).commit();
                Snacker.makeSnackBarForSpinner(mContext,"connection type set to Manually(UDP)");
                break;
            case Settings.ConnectionTypeTestFile: //Assets file
                mPrefConnectEdit.putString(mContext.getString(R.string.ConnectionType),""+Settings.ConnectionTypeTestFile).commit();
                Settings.ConnectionType =Settings.ConnectionTypeTestFile;
                fm.beginTransaction().replace(R.id.fragment_container, new MyTestFileFragment()).commit();
                Snacker.makeSnackBarForSpinner(mContext,"connection type set to Test file (Assets) ");
                break;
            case Settings.ConnectionTypeStorageFile:
                mPrefConnectEdit.putString(mContext.getString(R.string.ConnectionType),""+Settings.ConnectionTypeStorageFile).commit();
                Settings.ConnectionType =Settings.ConnectionTypeStorageFile;
                fm.beginTransaction().replace(R.id.fragment_container, new MyGroundRecFileFragment()).commit();
                Snacker.makeSnackBarForSpinner(mContext,"connection type set to GRecFile (File)");
                break;
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {

    }

    private void makeInfoDialog(final String message){
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                //android.support.v7.app.AlertDialog.Builder builder = new android.support.v7.app.AlertDialog.Builder(mContext);
                //AlertDialog.Builder builder = new AlertDialog.Builder(mContext,R.style.WrapEverythingDialog);
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setMessage(message);
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }
}
