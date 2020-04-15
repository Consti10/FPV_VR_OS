package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;

import java.io.File;

import constantin.fpv_vr.R;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.VideoPlayer.VideoSettings;


public class FConnectGroundRecFile extends Fragment {
    private Context mContext;
    //private EditText editTextTelemetry;
    //private EditText editTextVideo;
    private EditText editTextFileNameFPV;


    @SuppressLint("SetTextI18n")
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext =getActivity();
        View rootView = inflater.inflate(R.layout.connect_grfile_fragment, container, false);
        editTextFileNameFPV=rootView.findViewById(R.id.editTextFileNameFPV);
        editTextFileNameFPV.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                final String input=v.getText().toString();
                if(v.equals(editTextFileNameFPV)){
                    final String pathAndFilename=VideoSettings.getDirectoryToSaveDataTo()+input;
                    if(!fileExists(pathAndFilename)){
                        makeInfoDialog("WARNING ! This video file does not exist.");
                    }
                    VideoSettings.setVS_PLAYBACK_FILENAME(mContext,pathAndFilename);
                    TelemetrySettings.setT_PLAYBACK_FILENAME(mContext,pathAndFilename);
                }
                return false;
            }
        });
        //We need to write the filename double - once for video, once for telemetry lib.
        final String filenameFPV=extractFilename(VideoSettings.getVS_PLAYBACK_FILENAME(mContext));
        editTextFileNameFPV.setText(filenameFPV);
        final Button bEasySelectFPV = rootView.findViewById(R.id.bEasySelectFileFPV);
        bEasySelectFPV.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String directory=VideoSettings.getDirectoryToSaveDataTo();
                final String[] filenames=getAllFilenamesInDirectory(directory);
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setTitle("Pick a ground recording file");
                builder.setItems(filenames, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        final String selectedFilename=filenames[which];
                        editTextFileNameFPV.setText(selectedFilename);
                        VideoSettings.setVS_PLAYBACK_FILENAME(mContext,directory+selectedFilename);
                        TelemetrySettings.setT_PLAYBACK_FILENAME(mContext,directory+selectedFilename);
                    }
                });
                builder.show();
            }
        });
        return rootView;
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

    private static boolean fileExists(String fileName){
        File tempFile = new File(fileName);
        return tempFile.exists();
    }

    private static String extractFilename(final String pathWithFilename){
        String last = pathWithFilename.substring(pathWithFilename.lastIndexOf('/') + 1);
        System.out.println(pathWithFilename);
        System.out.println(last);
        return last;
    }

    private static String[] getAllFilenamesInDirectory(final String directory){
        File folder = new File(directory);
        File[] listOfFiles = folder.listFiles();
        String[] ret=new String[listOfFiles.length];
        for(int i=0;i<listOfFiles.length;i++){
            final File file=listOfFiles[i];
            final String filename=file.getName();
            System.out.println(filename);
            ret[i]=filename;
        }
        return ret;
    }



}
