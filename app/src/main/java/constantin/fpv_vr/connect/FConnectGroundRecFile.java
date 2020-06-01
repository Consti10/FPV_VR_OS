package constantin.fpv_vr.connect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;

import java.io.File;
import java.util.ArrayList;

import constantin.fpv_vr.databinding.ConnectGrfileFragmentBinding;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.video_player.VideoSettings;


public class FConnectGroundRecFile extends Fragment {
    private Context mContext;
    private ConnectGrfileFragmentBinding binding;

    @SuppressLint("SetTextI18n")
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext =getActivity();
        binding=ConnectGrfileFragmentBinding.inflate(inflater);
        binding.editTextFileNameFPV.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                final String input=v.getText().toString();
                if(v.equals(binding.editTextFileNameFPV)){
                    final String pathAndFilename=VideoSettings.getDirectoryToSaveDataTo()+input;
                    if(!FileHelper.fileExists(pathAndFilename)){
                        makeInfoDialog("WARNING ! This video file does not exist.");
                    }
                    VideoSettings.setVS_PLAYBACK_FILENAME(mContext,pathAndFilename);
                    TelemetrySettings.setT_PLAYBACK_FILENAME(mContext,pathAndFilename);
                }
                return false;
            }
        });
        //We need to write the filename double - once for video, once for telemetry lib.
        final String filenameFPV=FileHelper.extractFilename(VideoSettings.getVS_PLAYBACK_FILENAME(mContext));
        binding.editTextFileNameFPV.setText(filenameFPV);
        binding.bEasySelectFileFPV.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String directory= VideoSettings.getDirectoryToSaveDataTo();
                final ArrayList<String> filenames=FileHelper.getAllFilenamesInDirectory(directory,null);
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setTitle("Pick a ground recording file");
                builder.setItems(filenames.toArray(new String[0]), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        final String selectedFilename=filenames.get(which);
                        binding.editTextFileNameFPV.setText(selectedFilename);
                        VideoSettings.setVS_PLAYBACK_FILENAME(mContext,directory+selectedFilename);
                        TelemetrySettings.setT_PLAYBACK_FILENAME(mContext,directory+selectedFilename);
                    }
                });
                builder.show();
            }
        });
        return binding.getRoot();
    }


    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }


    private void makeInfoDialog(final String message){
        ((Activity) mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                androidx.appcompat.app.AlertDialog.Builder builder = new androidx.appcompat.app.AlertDialog.Builder(mContext);
                builder.setMessage(message);
                androidx.appcompat.app.AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }



}
