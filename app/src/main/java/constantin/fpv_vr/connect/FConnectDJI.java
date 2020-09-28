package constantin.fpv_vr.connect;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import constantin.fpv_vr.Toaster;
import constantin.fpv_vr.databinding.ConnectDjiFragmentBinding;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.fpv_vr.djiintegration.DJIHelper;
import constantin.video.core.RequestPermissionHelper;
import dji.common.airlink.SignalQualityCallback;
import dji.common.airlink.WiFiFrequencyBand;
import dji.common.error.DJIError;
import dji.common.util.CommonCallbacks;
import dji.sdk.airlink.AirLink;
import dji.sdk.airlink.WiFiLink;
import dji.sdk.flightcontroller.FlightController;
import dji.sdk.products.Aircraft;
import dji.sdk.remotecontroller.RemoteController;

public class FConnectDJI extends Fragment implements View.OnClickListener, RequestPermissionHelper.IOnPermissionsGranted{
    private static final String TAG=FConnectDJI.class.getSimpleName();
    private ConnectDjiFragmentBinding binding;
    private Context mContext;
    private final RequestPermissionHelper requestPermissionHelper=new RequestPermissionHelper(new String[]{
            Manifest.permission.VIBRATE, // Gimbal rotation
            Manifest.permission.INTERNET, // API requests
            Manifest.permission.ACCESS_WIFI_STATE, // WIFI connected products
            Manifest.permission.ACCESS_COARSE_LOCATION, // Maps
            Manifest.permission.ACCESS_NETWORK_STATE, // WIFI connected products
            Manifest.permission.ACCESS_FINE_LOCATION, // Maps
            Manifest.permission.CHANGE_WIFI_STATE, // Changing between WIFI and USB connection
            Manifest.permission.WRITE_EXTERNAL_STORAGE, // Log files
            Manifest.permission.BLUETOOTH, // Bluetooth connected products
            Manifest.permission.BLUETOOTH_ADMIN, // Bluetooth connected products
            Manifest.permission.READ_EXTERNAL_STORAGE, // Log files
            Manifest.permission.READ_PHONE_STATE, // Device UUID accessed upon registration
            Manifest.permission.RECORD_AUDIO // Speaker accessory
    },this);

    private final ArrayList<String> debugList=new ArrayList<>();

    private Timer updateTimer;

    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();
        binding=ConnectDjiFragmentBinding.inflate(inflater);
        //requestPermissionHelper.checkAndRequestPermissions(requireActivity());
        for(int i=0;i<15;i++){
            debugList.add("X");
        }

        final Aircraft aircraft=DJIApplication.getConnectedAircraft();
        if(aircraft!=null) {
            Toaster.makeToast(requireContext(),"Has aircraft");
            populateDebugList(aircraft);
        }else{
            Toaster.makeToast(requireContext(),"Has no aircraft");
        }
        setTextDebug();
        return binding.getRoot();
    }

    private void populateDebugList(final @NonNull Aircraft aircraft){
        aircraft.getName(callbackString(0,"Aircraft name"));

        debugList.set(1,"DisplayName "+aircraft.getModel().getDisplayName());
        debugList.set(2,"FirmwarePackageVersion "+aircraft.getFirmwarePackageVersion());
        final FlightController flightController=aircraft.getFlightController();
        flightController.getMaxFlightHeight(callbackInt(3,"Max flight height"));

        final AirLink airLink=aircraft.getAirLink();
        debugList.set(4,"airLink.isConnected "+airLink.isConnected());
        debugList.set(5,"isWiFiLinkSupported "+airLink.isWiFiLinkSupported());
        debugList.set(6,"isLightbridgeLinkSupported "+airLink.isLightbridgeLinkSupported());
        debugList.set(7,"isOcuSyncLinkSupported "+airLink.isOcuSyncLinkSupported());
        debugList.set(8,"isUpdateCountryCodeRequired() "+airLink.isUpdateCountryCodeRequired());

        airLink.setDownlinkSignalQualityCallback(callbackSignal(9,"DownlinkSignalQuality"));
        airLink.setUplinkSignalQualityCallback(callbackSignal(10,"UplinkSignalQuality"));
        final WiFiLink wiFiLink=airLink.getWiFiLink();
        wiFiLink.getFrequencyBand(callbackWifi(11,"Wifi frequency band"));
        wiFiLink.getChannelNumber(callbackInt(12,"Channel number"));

        final RemoteController remoteController=aircraft.getRemoteController();
        remoteController.getName(callbackString(13,"Remote controller name"));
    }

    private CommonCallbacks.CompletionCallbackWith<String> callbackString(int idx, String message){
        return new CommonCallbacks.CompletionCallbackWith<String>() {
            @Override
            public void onSuccess(String s) {
                debugList.set(idx,message+" "+s);
            }

            @Override
            public void onFailure(DJIError djiError) {
                debugList.set(idx,message+" "+DJIHelper.asString(djiError));
            }
        };
    }

    private CommonCallbacks.CompletionCallbackWith<Integer> callbackInt(int idx, String message){
        return new CommonCallbacks.CompletionCallbackWith<Integer>() {
            @Override
            public void onSuccess(Integer integer) {
                debugList.set(idx,message+" "+integer.toString());
            }

            @Override
            public void onFailure(DJIError djiError) {
                debugList.set(idx,message+" "+DJIHelper.asString(djiError));
            }
        };
    }

    private CommonCallbacks.CompletionCallbackWith<WiFiFrequencyBand> callbackWifi(int idx,String message){
        return new CommonCallbacks.CompletionCallbackWith<WiFiFrequencyBand>() {
            @Override
            public void onSuccess(WiFiFrequencyBand wiFiFrequencyBand) {
                debugList.set(idx,message+" "+DJIHelper.frequencyBandToString(wiFiFrequencyBand));
            }

            @Override
            public void onFailure(DJIError djiError) {
                debugList.set(idx,message+" "+DJIHelper.asString(djiError));
            }
        };
    }

    private SignalQualityCallback callbackSignal(int idx,String message){
        return new SignalQualityCallback() {
            @Override
            public void onUpdate(int i) {
                debugList.set(idx,message+" "+i);
            }
        };
    }

    
    private void setTextDebug(){
        StringBuilder content= new StringBuilder();
        for(final String s:debugList){
            content.append(s);
            content.append("\n");
        }
        binding.djiDebug.setText(content.toString());
    }

    @Override
    public void onResume() {
        super.onResume();
        requestPermissionHelper.checkAndRequestPermissions(requireActivity());
        //if(requestPermissionHelper.allPermissionsGranted(requireActivity())){
        //    final Application application=requireActivity().getApplication();
        //    ((DJIApplication)application).initializeDJIIfNeeded();
        //}
        updateTimer=new Timer();
        updateTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                requireActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.d(TAG,"update ui");
                        setTextDebug();
                    }
                });
            }
        }, 0, 500);
    }

    @Override
    public void onPause() {
        super.onPause();
        updateTimer.cancel();
        updateTimer.purge();
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
    }


    @SuppressLint("SetTextI18n")
    @Override
    public void onClick(View v) {
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        requestPermissionHelper.onRequestPermissionsResult(requestCode,permissions,grantResults);
    }

    @Override
    public void onPermissionsGranted() {
        final Application application=requireActivity().getApplication();
        ((DJIApplication)application).initializeDJIIfNeeded();
    }
}
