package constantin.fpv_vr;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;

import java.io.FileNotFoundException;
import java.io.IOException;

import static android.content.Context.MODE_PRIVATE;


public class MyGroundRecFileFragment extends Fragment implements TextView.OnEditorActionListener{
    private Context mConetxt;
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mConetxt=getActivity();
        View rootView = inflater.inflate(R.layout.my_grfile_fragment, container, false);
        EditText ETFileNameVideoSource=rootView.findViewById(R.id.editTextFileNameVideoSource);
        ETFileNameVideoSource.setOnEditorActionListener(this);
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

    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        //The user edited the FileNameVideoSource text view string
        //Check if the file exists. If not, make a warning Dialog
        if(!fileExists(v.getText().toString())){
            makeInfoDialog("WARNING ! This file does not exist.");
        }
        mConetxt.getSharedPreferences("pref_connect", MODE_PRIVATE).edit().putString(getString(R.string.GroundRecFileName),v.getText().toString()).commit();
        return false;
    }

    private void makeInfoDialog(final String message){
        ((Activity)mConetxt).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                android.support.v7.app.AlertDialog.Builder builder = new android.support.v7.app.AlertDialog.Builder(mConetxt);
                builder.setMessage(message);
                android.support.v7.app.AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }

    public static boolean fileExists(String fileName){
        try {
            java.io.FileInputStream in;
            in=new java.io.FileInputStream(Environment.getExternalStorageDirectory()+"/"+fileName);
            try {in.close();} catch (IOException e) {e.printStackTrace();}
            return true;
        } catch (FileNotFoundException e) {
            return false;
        }
    }

}
