package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import constantin.fpv_vr.R;
import constantin.fpv_vr.Toaster;
import constantin.fpv_vr.databinding.ConnectEzwbFragmentBinding;
import constantin.telemetry.core.TestReceiverTelemetry;
import constantin.video.core.IsConnected;
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
    private ConnectEzwbFragmentBinding binding;


    private OpenHDConnectionListener mOpenHDConnectionListener;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();
        binding=ConnectEzwbFragmentBinding.inflate(inflater);
        binding.Mode1ConnectB.setOnClickListener(this);
        binding.Mode2ConnectB.setOnClickListener(this);
        binding.Mode1InfoB.setOnClickListener(this);
        binding.Mode2InfoB.setOnClickListener(this);
        final FragmentActivity activity=requireActivity();
        mOpenHDConnectionListener=new OpenHDConnectionListener(activity,this);
        mTestReceiverTelemetry =new TestReceiverTelemetry(activity);
        mTestReceiverVideo=new TestReceiverVideo(activity);

        return binding.getRoot();
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
            binding.Mode1StatusTV.setText("Status: Connected");
            binding.Mode1StatusTV.setTextColor(Color.argb(255,0,255,0));
            binding.Mode1ConnectB.setText("Disconnect");
        }else{
            binding.Mode1StatusTV.setText("Status: Not connected");
            binding.Mode1StatusTV.setTextColor(Color.argb(255,255,51,51));
            binding.Mode1ConnectB.setText("Connect");
        }
        switch (currUSBStatus){
            case TETHERING:
                binding.Mode2StatusTV.setText("Status: Connected");
                binding.Mode2StatusTV.setTextColor(Color.argb(255,0,255,0));
                binding.Mode2ConnectB.setText("Disconnect");
                break;
            case DATA:
                binding.Mode2StatusTV.setText("Status: No Tethering");
                binding.Mode2StatusTV.setTextColor(Color.argb(255,255,51,51));
                binding.Mode2ConnectB.setText("Connect");
                break;
            case NOTHING:
                binding.Mode2StatusTV.setText("Status: No USB Connection");
                binding.Mode2StatusTV.setTextColor(Color.argb(255,255,51,51));
                binding.Mode2ConnectB.setText("Connect");
                break;
        }
        //start the test receiver if ether WIFI or USB is connected
        if(wifiConnectedToSystem || usbConnectedToSystem){
            //The test receiver also changes the text view to the right color
            mTestReceiverTelemetry.setViews(binding.FEZWBReceivedTelemetryDataTV,binding.FEZWBReceivedEZWBForwardDataTV,null);
            mTestReceiverVideo.setViews(binding.FEZWBReceivedVideoDataTV,null);
        }else{
            mTestReceiverTelemetry.setViews(null,null,null);
            mTestReceiverVideo.setViews(null,null);
            binding.FEZWBReceivedVideoDataTV.setText("No receiver detected.\nRX:0");
            binding.FEZWBReceivedVideoDataTV.setTextColor(Color.argb(255,255,0,0));
            binding.FEZWBReceivedTelemetryDataTV.setText("");
            binding.FEZWBReceivedEZWBForwardDataTV.setText("");
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
