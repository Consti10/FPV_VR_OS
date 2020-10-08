package constantin.fpv_vr.connect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;

import java.util.ArrayList;

import constantin.fpv_vr.databinding.ConnectUvcFragmentBinding;
import constantin.fpv_vr.settings.SJ;
import constantin.uvcintegration.TranscodeService;
import constantin.uvcintegration.UVCReceiverDecoder;

public class FConnectUVC extends Fragment implements View.OnClickListener{
    private ConnectUvcFragmentBinding binding;
    private Context mContext;

    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=requireActivity();
        binding=ConnectUvcFragmentBinding.inflate(inflater);
        binding.bStartT.setOnClickListener(v -> {
            TranscodeService.startTranscoding(mContext,"");
        });
        binding.bStopT.setOnClickListener(v -> {
           TranscodeService.stopTranscoding(mContext);
        });
        binding.bStartT2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String directory= UVCReceiverDecoder.getDirectoryToSaveDataTo();
                final ArrayList<String> filenames=FileHelper.getAllFilenamesInDirectory(directory,".fpv");
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setTitle("Pick a ground recording file");
                builder.setItems(filenames.toArray(new String[0]), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        final String selectedFilename=filenames.get(which);
                        final String filePath=directory+selectedFilename;
                        TranscodeService.startTranscoding(mContext,filePath);
                        //final Thread thread=new Thread(new SimpleEncoder(filePath));
                        //thread.start();
                    }
                });
                builder.show();
            }
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
