package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import constantin.fpv_vr.R;
import constantin.telemetry.core.TelemetryReceiver;

import static android.content.Context.MODE_PRIVATE;


public class FConnectGroundRecFile extends Fragment implements TextView.OnEditorActionListener{
    private Context mContext;
    private EditText editTextTelemetry;
    private EditText editTextVideo;


    @SuppressLint("SetTextI18n")
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext =getActivity();
        createFoldersIfNotYetExisting();
        View rootView = inflater.inflate(R.layout.connect_grfile_fragment, container, false);
        editTextVideo=rootView.findViewById(R.id.editTextFileNameVideoSource);
        editTextVideo.setOnEditorActionListener(this);
        editTextTelemetry=rootView.findViewById(R.id.editTextFileTelemetrySource);
        editTextTelemetry.setOnEditorActionListener(this);

        final String filenameVideo=getFilenameVideo(mContext);
        final String filenameTelemetry=getFilenameTelemetry(mContext);
        editTextVideo.setText(filenameVideo);
        editTextTelemetry.setText(filenameTelemetry);

        Button bEasySelectVideo = rootView.findViewById(R.id.bEasySelectVideo);
        bEasySelectVideo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String[] filenames=getAllFilenamesInDirectory(getDirectory()+"Video/");
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setTitle("Pick a video file");
                builder.setItems(filenames, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // the user clicked on colors[which]
                        final String selectedFilename=filenames[which];
                        editTextVideo.setText(selectedFilename);
                        setFilenameVideo(mContext,getDirectory()+"Video/"+selectedFilename);
                    }
                });
                builder.show();
            }
        });
        Button bEasySelectTelemetry=rootView.findViewById(R.id.bEasySelectTelemetry);
        bEasySelectTelemetry.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String[] filenames=getAllFilenamesInDirectory(getDirectory()+"Telemetry/");
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setTitle("Pick a telemetry file");
                builder.setItems(filenames, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // the user clicked on colors[which]
                        final String selectedFilename=filenames[which];
                        editTextTelemetry.setText(selectedFilename);
                        setFilenameTelemetry(mContext,getDirectory()+"Telemetry/"+selectedFilename);
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
        warnUserIfMismatchProtocol();
    }

    @SuppressLint("ApplySharedPref")
    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        //The user edited the FileNameVideoSource text view string
        //Check if the file exists. If not, make a warning Dialog
        final String input=v.getText().toString();
        if(v.equals(editTextVideo)){
            final String pathAndFilename=getDirectory()+"Video/"+input;
            if(!fileExists(pathAndFilename)){
                makeInfoDialog("WARNING ! This video file does not exist.");
            }
            setFilenameVideo(mContext,pathAndFilename);
        }
        if(v.equals(editTextTelemetry)){
            final String pathAndFilename=getDirectory()+"Telemetry/"+input;
            if(!fileExists(pathAndFilename)){
                makeInfoDialog("WARNING ! This telemetry file does not exist.");
            }
            setFilenameTelemetry(mContext,pathAndFilename);
        }
        return false;
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

    private void warnUserIfMismatchProtocol(){
        if(!TelemetryReceiver.checkFileExtensionMatchTelemetryProtocol(mContext)){
            Toast.makeText(mContext,"WARNING ! Telemetry protocol does not match file",Toast.LENGTH_LONG).show();
        }
    }

    private static String getDirectory(){
        return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)+"/FPV_VR/";
    }

    private void createFoldersIfNotYetExisting(){
        final boolean mkdirs1 = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) +"/FPV_VR/").mkdirs();
        final boolean mkdirs2 = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) +"/FPV_VR/Telemetry/").mkdirs();
        final boolean mkdirs3 = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) +"/FPV_VR/Video/").mkdirs();
    }

    private static String extractFilename(final String pathWithFilename){
        String last = pathWithFilename.substring(pathWithFilename.lastIndexOf('/') + 1);
        System.out.println(pathWithFilename);
        System.out.println(last);
        return last;
    }


    private static String getFilenameVideo(final Context context){
        final String tmp=context.getSharedPreferences("pref_video",Context.MODE_PRIVATE).
                getString(context.getString(R.string.VS_PLAYBACK_FILENAME),context.getString(R.string.VS_PLAYBACK_FILENAME_DEFAULT_VALUE));
        return extractFilename(tmp);
    }

    private static String getFilenameTelemetry(final Context context){
        final String tmp=context.getSharedPreferences("pref_telemetry",Context.MODE_PRIVATE).
                getString(context.getString(R.string.T_PLAYBACK_FILENAME), context.getString(R.string.T_PLAYBACK_FILENAME_DEFAULT_VALUE));
        return extractFilename(tmp);
    }


    @SuppressLint("ApplySharedPref")
    private static void setFilenameVideo(final Context context, final String pathAndFilename){
        context.getSharedPreferences("pref_video",Context.MODE_PRIVATE).edit().
                putString(context.getString(R.string.VS_PLAYBACK_FILENAME),pathAndFilename).commit();
    }

    @SuppressLint("ApplySharedPref")
    private static void setFilenameTelemetry(final Context context, final String pathAndFilename){
        context.getSharedPreferences("pref_telemetry",Context.MODE_PRIVATE).edit().
                putString(context.getString(R.string.T_PLAYBACK_FILENAME),pathAndFilename).commit();
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
