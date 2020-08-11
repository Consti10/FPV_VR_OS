package constantin.fpv_vr.connect;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;

import constantin.fpv_vr.R;
import constantin.fpv_vr.databinding.ConnectManuallyFragmentBinding;
import constantin.telemetry.core.TestReceiverTelemetry;
import constantin.video.core.IsConnected;
import constantin.video.core.TestReceiverVideo;


public class FConnectManually extends Fragment implements View.OnClickListener{
    private TestReceiverTelemetry mTestReceiverTelemetry;
    private TestReceiverVideo mTestReceiverVideo;
    private ConnectManuallyFragmentBinding binding;
    private Context mContext;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        binding=ConnectManuallyFragmentBinding.inflate(inflater);
        binding.ipAdressesTV.setText(IsConnected.getActiveInetAddresses());
        mContext=getActivity();
        FragmentActivity activity=requireActivity();
        mTestReceiverTelemetry=new TestReceiverTelemetry(activity);
        mTestReceiverTelemetry.setViews(binding.FMReceivedTelemetryDataTV,null,null);
        mTestReceiverVideo=new TestReceiverVideo(activity);
        mTestReceiverVideo.setViews(binding.FMReceivedVideoDataTV,null);
        binding.ManuallyInfoB.setOnClickListener(this);
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
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.ManuallyInfoB:
                makeInfoDialog(mContext.getString(R.string.ManuallyInfo));
                break;
            default:
                break;
        }
    }

    private void makeInfoDialog(final String message){
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setMessage(message);
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }
}
