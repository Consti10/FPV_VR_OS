package constantin.fpv_vr.connect;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;

import constantin.fpv_vr.databinding.ConnectDjiFragmentBinding;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.video.core.RequestPermissionHelper;

public class FConnectDJI extends Fragment implements View.OnClickListener, RequestPermissionHelper.IOnPermissionsGranted{
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

    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();
        binding=ConnectDjiFragmentBinding.inflate(inflater);
        //requestPermissionHelper.checkAndRequestPermissions(requireActivity());
        return binding.getRoot();
    }

    @Override
    public void onResume() {
        super.onResume();
        requestPermissionHelper.checkAndRequestPermissions(requireActivity());
        //if(requestPermissionHelper.allPermissionsGranted(requireActivity())){
        //    final Application application=requireActivity().getApplication();
        //    ((DJIApplication)application).initializeDJIIfNeeded();
        //}
    }

    @Override
    public void onPause() {
        super.onPause();
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
