package constantin.fpv_vr;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import java.util.Timer;
import java.util.TimerTask;

@SuppressLint("ApplySharedPref")
public class MyEZWBFragment extends Fragment {
    private Context mContext;
    private TestReceiver mTestReceiver;

    private Button M1ConnectB,M2ConnectB;
    private TextView M1StatusTV,M2StatusTV;
    private TextView EZWBTestReceiverTextView;

    //checks for changes in the connection status every 200ms.
    //This "check" has to run on the UI thread, since it might have to modify UI elements,
    //e.g. a connection status button. invokeConnectionCheck schedules a runnable on the UI thread every 200ms
    //private Timer invokeConnectionCheck;
    final Handler connectionCheckHandler=new Handler();
    Runnable connectionCheckRunnable;
    private volatile boolean disableWifiMessageAlreadySent=false;
    private volatile boolean doNotShowDisableWifiMessage=false;


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();
        View rootView = inflater.inflate(R.layout.my_ezwb_fragment, container, false);
        M1StatusTV=rootView.findViewById(R.id.Mode1StatusTV);
        M2StatusTV=rootView.findViewById(R.id.Mode2StatusTV);
        M1ConnectB=rootView.findViewById(R.id.Mode1ConnectB);
        M2ConnectB=rootView.findViewById(R.id.Mode2ConnectB);
        EZWBTestReceiverTextView=rootView.findViewById(R.id.EZWBTestReceiverTextView);
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
        boolean wifiConnectedToEZWB=IsConnected.checkWifiConnectedToEZWB(mContext);
        int currUSBStatus=IsConnected.checkTetheringConnectedToEZWB(mContext);
        boolean usbConnectedToEZWB=currUSBStatus==IsConnected.USB_TETHERING;
        //If we have both a WIFI and USB tethering connection, we should disable one of them
        final WifiManager wifiManager=(WifiManager)mContext.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        if(wifiManager!=null){
            if(wifiManager.isWifiEnabled() && usbConnectedToEZWB){
                //If the app is connected to EZ-WB via usb, we recommend to disable WIFI.
                //ether the user might have forgotten to turn off wifi -> turning off wifi increases range usw
                //or if there is a wifi AND usb connection to EZWB I don't know if it will still work
                if(!disableWifiMessageAlreadySent && !doNotShowDisableWifiMessage){
                    disableWifiMessageAlreadySent=true;
                    String message="Do you want to disable WIFI ? \n(Highly recommended when using a USB connection)";
                    android.support.v7.app.AlertDialog.Builder builder = new android.support.v7.app.AlertDialog.Builder(mContext);
                    builder.setMessage(message);
                    builder.setPositiveButton("YES", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            if(which==DialogInterface.BUTTON_POSITIVE){
                                wifiManager.setWifiEnabled(false);
                                double ts=System.currentTimeMillis();
                                while (wifiManager.isWifiEnabled() && ((System.currentTimeMillis()-ts)<2000)){
                                    //wait up to 2 seconds for wifi to turn off
                                }
                                disableWifiMessageAlreadySent=false;
                                dialog.dismiss();
                            }
                        }
                    });
                    builder.setNegativeButton("NO", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            if(which==DialogInterface.BUTTON_NEGATIVE){
                                doNotShowDisableWifiMessage=true;
                            }
                        }
                    });
                    builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                            disableWifiMessageAlreadySent=false;
                        }
                    });
                    android.support.v7.app.AlertDialog dialog = builder.create();
                    dialog.show();
                }
            }
        }
        if(wifiConnectedToEZWB){
            M1StatusTV.setText("Status: Connected");
            M1StatusTV.setTextColor(Color.argb(255,0,255,0));
            M1ConnectB.setText("Disconnect");
        }else{
            M1StatusTV.setText("Status: Not connected");
            M1StatusTV.setTextColor(Color.argb(255,255,51,51));
            M1ConnectB.setText("Connect");
        }
        switch (currUSBStatus){
            case IsConnected.USB_TETHERING:
                M2StatusTV.setText("Status: Connected");
                M2StatusTV.setTextColor(Color.argb(255,0,255,0));
                M2ConnectB.setText("Disconnect");
                break;
            case IsConnected.USB_CONNECTED:
                M2StatusTV.setText("Status: No Tethering");
                M2StatusTV.setTextColor(Color.argb(255,255,51,51));
                M2ConnectB.setText("Connect");
                break;
            case IsConnected.USB_NOTHING:
                M2StatusTV.setText("Status: No USB Connection");
                M2StatusTV.setTextColor(Color.argb(255,255,51,51));
                M2ConnectB.setText("Connect");
                break;
        }
        //start the test receiver if ether WIFI or USB is connected
        if(wifiConnectedToEZWB || usbConnectedToEZWB){
            //The test receiver also changes the text view to the right color
            startTestReceiverIfNotAlreadyRunning(EZWBTestReceiverTextView);
        }else{
            stopTestReceiverIfRunning();
            EZWBTestReceiverTextView.setText("No receiver detected.\nRX:0");
            EZWBTestReceiverTextView.setTextColor(Color.argb(255,255,0,0));
        }
    }

    private synchronized void startTestReceiverIfNotAlreadyRunning(TextView tv){
        if(mTestReceiver==null){
            mTestReceiver=new TestReceiver(mContext,tv);
        }
    }

    private synchronized void stopTestReceiverIfRunning(){
        if(mTestReceiver!=null){
            mTestReceiver.stopReceiving();
            mTestReceiver=null;
        }
    }
}
