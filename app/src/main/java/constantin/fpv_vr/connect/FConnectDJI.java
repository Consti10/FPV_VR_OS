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
import androidx.fragment.app.Fragment;

import constantin.fpv_vr.databinding.ConnectDjiFragmentBinding;
import constantin.fpv_vr.djiintegration.DJIApplication;
import constantin.video.core.RequestPermissionHelper;

public class FConnectDJI extends Fragment implements View.OnClickListener, RequestPermissionHelper.IOnPermissionsGranted{
    private ConnectDjiFragmentBinding binding;
    private Context mContext;
    private final RequestPermissionHelper requestPermissionHelper=new RequestPermissionHelper(new String[]{
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.ACCESS_FINE_LOCATION,
            // Needed for DJI registering the SDK (alongside with WRITE_EXTERNAL_STORAGE)
            Manifest.permission.READ_PHONE_STATE,
    },this);

    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();
        binding=ConnectDjiFragmentBinding.inflate(inflater);
        requestPermissionHelper.checkAndRequestPermissions(requireActivity());
        return binding.getRoot();
    }

    @Override
    public void onResume() {
        super.onResume();
        if(requestPermissionHelper.allPermissionsGranted(requireActivity())){
            final Application application=requireActivity().getApplication();
            ((DJIApplication)application).initializeDJIIfNeeded();
        }
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
