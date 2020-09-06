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
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

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

import constantin.fpv_vr.databinding.ConnectGrfileFragmentBinding;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.video_player.VideoSettings;


public class FConnectGroundRecFile extends Fragment{
    private static final String TAG=FConnectGroundRecFile.class.getSimpleName();
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
                /*final String directory= VideoSettings.getDirectoryToSaveDataTo();
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
                builder.show();*/
                Intent intent = new Intent();
                intent.setAction(Intent.ACTION_OPEN_DOCUMENT);
                intent.setType("*/*");
                startActivityForResult(intent,99);
            }
        });
        binding.bDoSomething.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                final String directory= VideoSettings.getDirectoryToSaveDataTo();
                final ArrayList<String> filenames=FileHelper.getAllFilenamesInDirectory(directory,null);
                for(int i=0;i<filenames.size();i++){
                    //final String filename="TestFilename"+i;
                    final String filename=filenames.get(i);
                    final String filePath=directory+filename;

                    ContentResolver resolver = mContext.getContentResolver();
                    ContentValues contentValues = new ContentValues();
                    contentValues.put(MediaStore.Video.Media.RELATIVE_PATH, "Movies/" + "FPV_VR");
                    contentValues.put(MediaStore.Video.Media.TITLE, filename);
                    contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, filename);
                    contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "video/fpv");
                    Uri mUri = resolver.insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, contentValues);

                    try {
                        File sourceFile=new File(filePath);

                        FileInputStream inputStream=new FileInputStream(sourceFile);
                        FileDescriptor outputFD = resolver.openFileDescriptor(mUri, "rw").getFileDescriptor();
                        FileOutputStream outputStream=new FileOutputStream(outputFD);
                        copy(inputStream,outputStream);
                        Log.d(TAG,"Copied file "+inputStream.getFD().toString()+" to "+outputStream.getFD().toString());

                        //sourceFile.delete();

                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    //File outputFile=new File(mUri.getPath());
                    //copy(inputFile,outputFile);
                }
            }
        });
        binding.bDoSomething2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String directory= VideoSettings.getDirectoryToSaveDataTo();
                final ArrayList<String> filenames=FileHelper.getAllFilenamesInDirectory(directory,null);
                for(int i=0;i<filenames.size();i++) {
                    //final String filename="TestFilename"+i;
                    final String filename = filenames.get(i);
                    final String filePath = directory + filename;

                   addFileToContentProvider(filename,filePath);
                }
            }
        });
        return binding.getRoot();
    }

    @RequiresApi(api = Build.VERSION_CODES.Q)
    private void copyFilesToNewDirectory(){
        final String directory= VideoSettings.getDirectoryToSaveDataTo();
        final ArrayList<String> filenames=FileHelper.getAllFilenamesInDirectory(directory,null);
        for(int i=0;i<filenames.size();i++) {
            //final String filename="TestFilename"+i;
            final String filename = filenames.get(i);
            ContentResolver resolver = mContext.getContentResolver();
            ContentValues contentValues = new ContentValues();
            contentValues.put(MediaStore.Video.Media.RELATIVE_PATH, "Movies/" + "FPV_VR");
            contentValues.put(MediaStore.Video.Media.TITLE, filename);
            contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, filename);
            contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "video/fpv");
            Uri mUri = resolver.insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, contentValues);

            try {
                //File inputFile=new File(directory+filename);
                FileInputStream inputStream = new FileInputStream(directory + filename);
                FileDescriptor outputFD = resolver.openFileDescriptor(mUri, "rw").getFileDescriptor();
                FileOutputStream outputStream = new FileOutputStream(outputFD);
                copy(inputStream, outputStream);
                Log.d(TAG, "Copied file " + inputStream.getFD().toString() + " to " + outputStream.getFD().toString());
            } catch (IOException e) {
                e.printStackTrace();
            }
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


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
       super.onActivityResult(requestCode,resultCode,data);
       Log.d(TAG,"onActivityResult"+requestCode);
       if(data!=null){
           Uri selectedImageUri = data.getData();
           Log.d(TAG,"Got "+selectedImageUri.getPath()+" "+selectedImageUri.toString());
           File tmp=new File(selectedImageUri.getPath());
           Log.d(TAG,"from file "+tmp.getAbsoluteFile().getAbsolutePath());

           final String actualFilePath=FileHelper2.getRealPathFromURI_API19(mContext,selectedImageUri);
           Log.d(TAG,""+actualFilePath);
           VideoSettings.setVS_PLAYBACK_FILENAME(mContext,actualFilePath);
           /*try {
               ParcelFileDescriptor d = mContext.getContentResolver().openFileDescriptor(selectedImageUri,"r");
               //Log.d(TAG,""+d.get);
           } catch (IOException e) {
               e.printStackTrace();
           }*/

           //VideoSettings.setVS_PLAYBACK_FILENAME(mContext,tmp.g);
       }
    }


    private void addFileToContentProvider(final String filename,final String filePathAndName) {
        Log.d(TAG,"Add file "+filename+" "+filePathAndName);
        ContentValues values = new ContentValues();
        //values.put(MediaStore.Video.VideoColumns.DATE_ADDED, System.currentTimeMillis() / 1000);
        values.put(MediaStore.Video.Media.DISPLAY_NAME, "display_name");
        values.put(MediaStore.Video.Media.TITLE, "my_title");
        values.put(MediaStore.MediaColumns.MIME_TYPE, "video/fpv");
        //values.put(MediaStore.Video.Media.RELATIVE_PATH, "$Q_VIDEO_PATH/$relativePath")
        values.put(MediaStore.Video.Media.DATA,filePathAndName);

        Uri uri=mContext.getContentResolver().insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, values);
        if(uri!=null){
            Log.d(TAG,"URI "+uri.toString());
        }else{
            Log.d(TAG,"URL is null - something went wrong");
        }
        //sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.parse("file://"+path+filename)));
    }
}
