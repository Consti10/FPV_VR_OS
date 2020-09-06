package constantin.fpv_vr.connect;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.fragment.app.Fragment;

import com.hbisoft.pickit.PickiT;
import com.hbisoft.pickit.PickiTCallbacks;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;

import constantin.fpv_vr.Toaster;
import constantin.fpv_vr.databinding.ConnectGrfileFragmentBinding;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.video_player.VideoSettings;


public class FConnectGroundRecFile extends Fragment{
    private static final String TAG=FConnectGroundRecFile.class.getSimpleName();
    private Context mContext;
    private ConnectGrfileFragmentBinding binding;
    private static final int REQUEST_CODE_PICK_FILE=55;

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
                Intent intent = new Intent();
                intent.setAction(Intent.ACTION_OPEN_DOCUMENT);
                intent.setType("video/*");
                Toaster.makeToast(mContext,"Select .fpv ground recording file");
                startActivityForResult(intent,REQUEST_CODE_PICK_FILE);
            }
        });
        binding.bMigrateFiles.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    copyFilesToNewDirectory();
                }
            }
        });
        return binding.getRoot();
    }

    @RequiresApi(api = Build.VERSION_CODES.Q)
    private void copyFilesToNewDirectory(){
        final String directory= Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)+"/FPV_VR/";
        final ArrayList<String> filenames=FileHelper.getAllFilenamesInDirectory(directory,null);
        for(int i=0;i<filenames.size();i++) {
            //final String filename="TestFilename"+i;
            final String filename = filenames.get(i);
            final String filePath = directory + filename;
            ContentResolver resolver = mContext.getContentResolver();
            ContentValues contentValues = new ContentValues();
            contentValues.put(MediaStore.Video.Media.RELATIVE_PATH, "Movies/" + "FPV_VR");
            contentValues.put(MediaStore.Video.Media.TITLE, filename);
            contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, filename);
            contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "video/fpv");
            Uri mUri = resolver.insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, contentValues);
            try {
                File sourceFile = new File(filePath);
                FileInputStream inputStream = new FileInputStream(sourceFile);
                FileDescriptor outputFD = resolver.openFileDescriptor(mUri, "rw").getFileDescriptor();
                FileOutputStream outputStream = new FileOutputStream(outputFD);
                copy(inputStream, outputStream);
                Log.d(TAG, "Copied file " + inputStream.getFD().toString() + " to " + outputStream.getFD().toString());
                //sourceFile.delete();
            } catch (IOException e) {
                e.printStackTrace();
            }
            //File outputFile=new File(mUri.getPath());
            //copy(inputFile,outputFile);
        }
    }

    public static void copy(FileInputStream in,FileOutputStream out) throws IOException{
        // Transfer bytes from in to out
        byte[] buf = new byte[1024];
        int len;
        while ((len = in.read(buf)) > 0) {
            out.write(buf, 0, len);
        }
    }

    public static void copy(File src, File dst) {
        try (InputStream in = new FileInputStream(src)) {
            try (OutputStream out = new FileOutputStream(dst)) {
                // Transfer bytes from in to out
                byte[] buf = new byte[1024];
                int len;
                while ((len = in.read(buf)) > 0) {
                    out.write(buf, 0, len);
                }
            }
        }catch (IOException e){
            e.printStackTrace();
        }
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
        ((Activity) mContext).runOnUiThread(() -> {
            androidx.appcompat.app.AlertDialog.Builder builder = new androidx.appcompat.app.AlertDialog.Builder(mContext);
            builder.setMessage(message);
            androidx.appcompat.app.AlertDialog dialog = builder.create();
            dialog.show();
        });
    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
       super.onActivityResult(requestCode,resultCode,data);
       //Log.d(TAG,"onActivityResult"+requestCode);
       if(requestCode==REQUEST_CODE_PICK_FILE && data!=null){
           Uri selectedImageUri = data.getData();
           Log.d(TAG,"Got "+selectedImageUri.getPath()+" "+selectedImageUri.toString());
           final String actualFilePath=FileHelper2.getRealPathFromURI_API19(mContext,selectedImageUri);
           if(actualFilePath!=null){
               //Log.d(TAG,"Got real path"+actualFilePath);
               if(actualFilePath.endsWith(".fpv")){
                   Log.d(TAG,"Got real path and file is fpv "+actualFilePath);
                   VideoSettings.setVS_PLAYBACK_FILENAME(mContext,actualFilePath);
                   TelemetrySettings.setT_PLAYBACK_FILENAME(mContext,actualFilePath);
                   Toaster.makeToast(mContext,"Selected .fpv ground recording file");
               }
           }
       }
    }

}
