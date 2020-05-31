package constantin.fpv_vr.connect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;

import constantin.fpv_vr.databinding.ConnectUvcFragmentBinding;
import constantin.fpv_vr.settings.SJ;
import constantin.test.SimpleEncoder;
import constantin.test.UVCReceiverDecoder;

public class FConnectUVC extends Fragment implements View.OnClickListener{
    private ConnectUvcFragmentBinding binding;
    private Context mContext;
    private long p;

    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=requireActivity();
        if(!SJ.OSD_DISABLE_ALL_OVERLAY_ELEMENTS(mContext)){
            final String message="Do you want to disable digital OSD elements that overlay the video ?" +
                    "You can always do that later manually via OSD Settings->OSD Elements->Other";

            new AlertDialog.Builder(mContext).setMessage(message)
                    .setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    SJ.OSD_DISABLE_ALL_OVERLAY_ELEMENTS(mContext,true);
                }
            }).setNegativeButton("No",null).show();
        }
        binding=ConnectUvcFragmentBinding.inflate(inflater);
        binding.bStartT.setOnClickListener(v -> {
            p= SimpleEncoder.nativeStartConvertFile(UVCReceiverDecoder.getDirectoryToSaveDataTo());
        });
        binding.bStopT.setOnClickListener(v -> {
            SimpleEncoder.nativeStopConvertFile(p);
        });
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
    public void onDestroy(){
        super.onDestroy();
    }


    @SuppressLint("SetTextI18n")
    @Override
    public void onClick(View v) {

    }

}
