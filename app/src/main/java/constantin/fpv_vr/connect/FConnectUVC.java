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
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;

import java.util.ArrayList;

import constantin.fpv_vr.databinding.ConnectUvcFragmentBinding;
import constantin.fpv_vr.settings.SJ;
import constantin.telemetry.core.TelemetrySettings;
import constantin.test.SimpleEncoder;
import constantin.test.UVCReceiverDecoder;
import constantin.video.core.video_player.VideoSettings;

public class FConnectUVC extends Fragment implements View.OnClickListener{
    private ConnectUvcFragmentBinding binding;
    private Context mContext;

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
            //thread=new Thread(new SimpleTranscoder(UVCReceiverDecoder.getDirectoryToSaveDataTo()+"TestInput.fpv"));
            Intent serviceIntent = new Intent(mContext, TranscodeService.class);
            serviceIntent.putExtra(TranscodeService.EXTRA_START_TRANSCODING_FILE, UVCReceiverDecoder.getDirectoryToSaveDataTo()+"TestInput.fpv");
            ContextCompat.startForegroundService(mContext, serviceIntent);
        });
        binding.bStopT.setOnClickListener(v -> {
            Intent serviceIntent = new Intent(mContext, TranscodeService.class);
            //serviceIntent.putExtra(TranscodeService.EXTRA_STOP_TRANSCODING_FILE, "STOP X");
            //requireActivity().startService(serviceIntent);
            requireActivity().stopService(serviceIntent);
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
                        final Thread thread=new Thread(new SimpleEncoder(filePath));
                        thread.start();
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
