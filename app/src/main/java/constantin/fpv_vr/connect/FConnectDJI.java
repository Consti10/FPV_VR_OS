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

import com.google.android.gms.common.util.ArrayUtils;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import constantin.fpv_vr.Permissions;
import constantin.fpv_vr.databinding.ConnectDjiFragmentBinding;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.fpv_vr.djiintegration.DJIHelper;
import constantin.video.core.RequestPermissionHelper;
import dji.common.airlink.SignalQualityCallback;
import dji.common.airlink.WiFiFrequencyBand;
import dji.common.airlink.WifiChannelInterference;
import dji.common.camera.SettingsDefinitions;
import dji.common.error.DJIError;
import dji.common.remotecontroller.HardwareState;
import dji.common.util.CommonCallbacks;
import dji.midware.data.model.P3.C;
import dji.sdk.airlink.AirLink;
import dji.sdk.airlink.WiFiLink;
import dji.sdk.camera.Camera;
import dji.sdk.flightcontroller.FlightController;
import dji.sdk.products.Aircraft;
import dji.sdk.remotecontroller.RemoteController;
import dji.sdk.sdkmanager.DJISDKManager;

public class FConnectDJI extends Fragment implements View.OnClickListener, RequestPermissionHelper.IOnPermissionsGranted{
    private static final String TAG=FConnectDJI.class.getSimpleName();
    private ConnectDjiFragmentBinding binding;
    private Context mContext;
    private final RequestPermissionHelper requestPermissionHelper=new RequestPermissionHelper(Permissions.DJI_PERMISSIONS,this);
    private final ArrayList<String> debugList=new ArrayList<>();

    private Timer updateTimer;

    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();
        binding=ConnectDjiFragmentBinding.inflate(inflater);
        //requestPermissionHelper.checkAndRequestPermissions(requireActivity());
        for(int i=0;i<25;i++){
            debugList.add("");
        }
        setTextDebug();
        return binding.getRoot();
    }

    private void populateDebugList(final @NonNull Aircraft aircraft){
        int idx=0;
        aircraft.getName(callbackGeneric(idx++,"Aircraft name"));

        debugList.set(idx++,"DisplayName "+aircraft.getModel().getDisplayName());
        debugList.set(idx++,"FirmwarePackageVersion "+aircraft.getFirmwarePackageVersion());
        final FlightController flightController=aircraft.getFlightController();
        flightController.getMaxFlightHeight(callbackGeneric(idx++,"Max flight height"));
        flightController.getGoHomeHeightInMeters(callbackGeneric(idx++,"Go home height"));
        flightController.getConnectionFailSafeBehavior(callbackGeneric(idx++,"FailSafeBehaviour"));

        final AirLink airLink=aircraft.getAirLink();
        debugList.set(idx++,"airLink.isConnected "+airLink.isConnected());
        debugList.set(idx++,"isWiFiLinkSupported "+airLink.isWiFiLinkSupported());
        debugList.set(idx++,"isLightbridgeLinkSupported "+airLink.isLightbridgeLinkSupported());
        debugList.set(idx++,"isOcuSyncLinkSupported "+airLink.isOcuSyncLinkSupported());
        debugList.set(idx++,"isUpdateCountryCodeRequired() "+airLink.isUpdateCountryCodeRequired());


        airLink.setDownlinkSignalQualityCallback(callbackSignal(idx++,"DownlinkSignalQuality"));
        airLink.setUplinkSignalQualityCallback(callbackSignal(idx++,"UplinkSignalQuality"));
        final WiFiLink wiFiLink=airLink.getWiFiLink();
        wiFiLink.getFrequencyBand(callbackGeneric(idx++,"Wifi frequency band"));
        wiFiLink.getChannelNumber(callbackGeneric(idx++,"Channel number"));
        wiFiLink.getAvailableChannelNumbers(callbackWifiChannels(idx++,idx++));
        wiFiLink.getDataRate(callbackGeneric(idx++,"Wifi data rate"));
        wiFiLink.getSelectionMode(callbackGeneric(idx++,"Wifi selection mode"));
        wiFiLink.setChannelInterferenceCallback(new WiFiLink.ChannelInterferenceCallback() {
            @Override
            public void onUpdate(WifiChannelInterference[] wifiChannelInterferences) {

            }
        });

        final RemoteController remoteController=aircraft.getRemoteController();
        remoteController.getName(callbackGeneric(idx++,"Remote controller name"));
        //debugList.set(16,"LOG "+DJISDKManager.getInstance().getLogPath());
        remoteController.getPairingState(callbackGeneric(idx++,"CTRL pairing"));

        final Camera camera=aircraft.getCamera();
        debugList.set(idx++,DJIHelper.asString(camera.getCapabilities().videoResolutionAndFrameRateRange()));
        camera.getMode(callbackGeneric(idx++,"Camera mode"));
        camera.getExposureMode(callbackGeneric(idx++,"Camera exposure mode"));
        //
    }

    private <T> CommonCallbacks.CompletionCallbackWith<T> callbackGeneric(int idx, String message){
        return new CommonCallbacks.CompletionCallbackWith<T>() {
            @Override
            public void onSuccess(T t) {
                debugList.set(idx,message+" "+t.toString());
            }
            @Override
            public void onFailure(DJIError djiError) {
                debugList.set(idx,message+" "+DJIHelper.asString(djiError));
            }
        };
    }
    private CommonCallbacks.CompletionCallbackWith<Integer[]> callbackWifiChannels(final int idx1,final int idx2){
        return new CommonCallbacks.CompletionCallbackWith<Integer[]>() {
            @Override
            public void onSuccess(Integer[] integers) {
                StringBuilder s= new StringBuilder();
                for(final int i:integers){
                    s.append(i);
                    s.append(" ");
                }
                debugList.set(idx1,"Channels are "+s);
                final boolean isCEMode=ArrayUtils.contains(integers,13);
                debugList.set(idx2,"Is CE Mode "+isCEMode);
            }

            @Override
            public void onFailure(DJIError djiError) {
                debugList.set(idx1,"Cannot get channels");
                debugList.set(idx2,"Is CE Mode unknown");
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
        updateTimer=new Timer();
        updateTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                requireActivity().runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Log.d(TAG,"update ui");
                        final boolean hasProduct=DJIApplication.getConnecteBaseProduct()!=null;
                        final boolean hasAircraft=DJIApplication.getConnectedAircraft()!=null;
                        binding.djiInfo.setText("DJI in development. Product "+hasProduct+" Aircraft "+hasAircraft);
                        final Aircraft aircraft=DJIApplication.getConnectedAircraft();
                        if(aircraft!=null) {
                            //Toaster.makeToast(requireContext(),"Has aircraft");
                            populateDebugList(aircraft);
                        }else{
                            //Toaster.makeToast(requireContext(),"Has no aircraft");
                        }
                        setTextDebug();
                    }
                });
            }
        }, 0, 1000);
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
