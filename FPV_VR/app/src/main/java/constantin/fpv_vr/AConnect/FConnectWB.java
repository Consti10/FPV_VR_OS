package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import constantin.fpv_vr.R;
import constantin.video.core.IsConnected;
import constantin.fpv_vr.Toaster;
import constantin.telemetry.core.TestReceiverTelemetry;
import constantin.video.core.TestReceiverVideo;

/**
 * Connect to EZ-Wifibroadcast/OpenHD
 */

@SuppressLint("ApplySharedPref")
public class FConnectWB extends Fragment implements View.OnClickListener {
    private Context mContext;
    private TestReceiverTelemetry mTestReceiverTelemetry;
    private TestReceiverVideo mTestReceiverVideo;

    private Button M1ConnectB,M2ConnectB;
    private TextView M1StatusTV,M2StatusTV;
    private TextView receivedVideoDataTV;
    private TextView receivedTelemetryDataTV;
    private TextView receivedEZWBForwardDataTV;

    //checks for changes in the connection status every x ms
    //This "check" has to run on the UI thread, since it might have to modify UI elements,
    //e.g. a connection status button. invokeConnectionCheck schedules a runnable on the UI thread every 500ms
    private final Handler connectionCheckHandler=new Handler();
    private Runnable connectionCheckRunnable;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();

        View rootView = inflater.inflate(R.layout.connect_ezwb_fragment, container, false);
        M1StatusTV=rootView.findViewById(R.id.Mode1StatusTV);
        M2StatusTV=rootView.findViewById(R.id.Mode2StatusTV);
        M1ConnectB=rootView.findViewById(R.id.Mode1ConnectB);
        M1ConnectB.setOnClickListener(this);
        M2ConnectB=rootView.findViewById(R.id.Mode2ConnectB);
        M2ConnectB.setOnClickListener(this);
        Button infoB1=rootView.findViewById(R.id.Mode1InfoB);
        infoB1.setOnClickListener(this);
        Button infoB2=rootView.findViewById(R.id.Mode2InfoB);
        infoB2.setOnClickListener(this);
        receivedVideoDataTV =rootView.findViewById(R.id.FEZWB_ReceivedVideoDataTV);
        receivedTelemetryDataTV=rootView.findViewById(R.id.FEZWB_ReceivedTelemetryDataTV);
        receivedEZWBForwardDataTV =rootView.findViewById(R.id.FEZWB_ReceivedEZWBForwardDataTV);
        connectionCheckRunnable = new Runnable() {
            @Override
            public void run() {
                ((Activity)mContext).runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        //check the connection statuses and change any ui elements if needed
                        updateEZWBConnectionStatus();
                    }
                });
                connectionCheckHandler.postDelayed(this, 500);
            }
        };
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        connectionCheckHandler.postDelayed(connectionCheckRunnable, 500);
    }

    @Override
    public void onPause() {
        super.onPause();
        connectionCheckHandler.removeCallbacks(connectionCheckRunnable);
        stopTestReceiverIfRunning();
    }

    @SuppressLint("SetTextI18n")
    private void updateEZWBConnectionStatus(){
        //boolean wifiConnectedToEZWB= IsConnected.checkWifiConnectedEZWB(mContext);
        //boolean wifiConnectedToOpenHD=IsConnected.checkWifiConnectedOpenHD(mContext);
        final boolean wifiConnectedToSystem=IsConnected.checkWifiConnectedEZWB(mContext) ||
                IsConnected.checkWifiConnectedOpenHD(mContext);

        final IsConnected.USB_CONNECTION currUSBStatus=IsConnected.getUSBStatus(mContext);
        final boolean usbConnectedToSystem=currUSBStatus==IsConnected.USB_CONNECTION.TETHERING;

        if(wifiConnectedToSystem){
            M1StatusTV.setText("Status: Connected");
            M1StatusTV.setTextColor(Color.argb(255,0,255,0));
            M1ConnectB.setText("Disconnect");
        }else{
            M1StatusTV.setText("Status: Not connected");
            M1StatusTV.setTextColor(Color.argb(255,255,51,51));
            M1ConnectB.setText("Connect");
        }
        switch (currUSBStatus){
            case TETHERING:
                M2StatusTV.setText("Status: Connected");
                M2StatusTV.setTextColor(Color.argb(255,0,255,0));
                M2ConnectB.setText("Disconnect");
                break;
            case DATA:
                M2StatusTV.setText("Status: No Tethering");
                M2StatusTV.setTextColor(Color.argb(255,255,51,51));
                M2ConnectB.setText("Connect");
                break;
            case NOTHING:
                M2StatusTV.setText("Status: No USB Connection");
                M2StatusTV.setTextColor(Color.argb(255,255,51,51));
                M2ConnectB.setText("Connect");
                break;
        }
        //start the test receiver if ether WIFI or USB is connected
        if(wifiConnectedToSystem || usbConnectedToSystem){
            //The test receiver also changes the text view to the right color
            startTestReceiverIfNotAlreadyRunning();
        }else{
            stopTestReceiverIfRunning();
            receivedVideoDataTV.setText("No receiver detected.\nRX:0");
            receivedVideoDataTV.setTextColor(Color.argb(255,255,0,0));
            receivedTelemetryDataTV.setText("");
            receivedEZWBForwardDataTV.setText("");
        }
    }

    private synchronized void startTestReceiverIfNotAlreadyRunning(){
        if(mTestReceiverTelemetry ==null){
            mTestReceiverTelemetry =new TestReceiverTelemetry(mContext,receivedTelemetryDataTV,receivedEZWBForwardDataTV,null);
            mTestReceiverTelemetry.startReceiving();
        }
        if(mTestReceiverVideo==null){
            mTestReceiverVideo=new TestReceiverVideo(mContext,receivedVideoDataTV,null);
            mTestReceiverVideo.startReceiving();
        }
    }

    private synchronized void stopTestReceiverIfRunning(){
        if(mTestReceiverTelemetry !=null){
            mTestReceiverTelemetry.stopReceiving();
            mTestReceiverTelemetry =null;
        }
        if(mTestReceiverVideo!=null){
            mTestReceiverVideo.stopReceiving();
            mTestReceiverVideo=null;
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.Mode1InfoB:
                Toast.makeText(mContext,mContext.getString(R.string.EZWBMode1Info),Toast.LENGTH_LONG).show();
                break;
            case R.id.Mode2InfoB:
                Toast.makeText(mContext,mContext.getString(R.string.EZWBMode2Info),Toast.LENGTH_LONG).show();
                break;
            case R.id.Mode1ConnectB:
                Toaster.makeToast(mContext, "Connect to EZ-WifiBroadcast with PW=wifibraodcast or\nOpenHD with PW=wifiopenhd", true);
                startActivity(new Intent(android.provider.Settings.ACTION_WIFI_SETTINGS));
                break;
            case R.id.Mode2ConnectB:
                if (IsConnected.getUSBStatus(mContext)== IsConnected.USB_CONNECTION.NOTHING) {
                    Toast.makeText(mContext,"Please check your usb connection",Toast.LENGTH_SHORT).show();
                    return;
                }
                IsConnected.openUSBTetherSettings(mContext);
                break;
            default:
                break;
        }
    }


}
