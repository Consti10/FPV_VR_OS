package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

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
 * 'Connect' to EZ-Wifibroadcast/OpenHD
 * There is no real connection (since all is lossy UDP) but I use this term since most users are probably used to it
 * Here, 'Connected' means 'data is coming in'
 */

@SuppressLint("ApplySharedPref")
public class FConnectWB extends Fragment implements View.OnClickListener , OpenHDConnectionListener.IConnectionStatus {
    private Context mContext;
    private TestReceiverTelemetry mTestReceiverTelemetry;
    private TestReceiverVideo mTestReceiverVideo;

    private Button M1ConnectB,M2ConnectB;
    private TextView M1StatusTV,M2StatusTV;
    private TextView receivedVideoDataTV;
    private TextView receivedTelemetryDataTV;
    private TextView receivedEZWBForwardDataTV;


    private OpenHDConnectionListener mOpenHDConnectionListener;

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
        final FragmentActivity activity=requireActivity();
        mOpenHDConnectionListener=new OpenHDConnectionListener(activity,this);
        mTestReceiverTelemetry =new TestReceiverTelemetry(activity);
        mTestReceiverVideo=new TestReceiverVideo(activity);

        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    @SuppressLint("SetTextI18n")
    public void refreshConnectionSTatus(final boolean wifiConnectedToSystem,final IsConnected.USB_CONNECTION currUSBStatus) {
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
            mTestReceiverTelemetry.setViews(receivedTelemetryDataTV,receivedEZWBForwardDataTV,null);
            mTestReceiverVideo.setViews(receivedVideoDataTV,null);
        }else{
            mTestReceiverTelemetry.setViews(null,null,null);
            mTestReceiverVideo.setViews(null,null);
            receivedVideoDataTV.setText("No receiver detected.\nRX:0");
            receivedVideoDataTV.setTextColor(Color.argb(255,255,0,0));
            receivedTelemetryDataTV.setText("");
            receivedEZWBForwardDataTV.setText("");
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
