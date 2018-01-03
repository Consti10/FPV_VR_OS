package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.PowerManager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class Activity_FirstStart extends AppCompatActivity {
    private Context mContext;

    private boolean dialogToTheBeginningHasBeenShown =false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        setContentView(R.layout.activity_first_startup);
    }

    @Override
    protected void onResume(){
        super.onResume();
        SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
        boolean MSAATesterStart=pref_static.getBoolean(getString(R.string.A_MSAATester_Start),false);
        boolean MSAATesterStop=pref_static.getBoolean(getString(R.string.A_MSAATester_Stop),false);
        boolean FBRTesterStart=pref_static.getBoolean(getString(R.string.A_FBTester_Start),false);
        boolean FBRTesterStop=pref_static.getBoolean(getString(R.string.A_FBTester_Stop),false);
        if(!dialogToTheBeginningHasBeenShown){
            String text="FPV_VR needs to detect the CPU and GPU abilities of your phone once to adjust the power and performance settings. ";
            if(MSAATesterStart&&!MSAATesterStop){
                text="MSAA config testing failed. No MSAA available";
            }
            if(FBRTesterStart&&!FBRTesterStop){
                text="FBR tester failed. No FBR possible";
            }
            makeStartDialog(text);
            return;
        }
        if(MSAATesterStart){
            if(MSAATesterStop){
                //MSAA has been successfully tested
            }else{
                Activity_TestMSAA.OnCrash(this);
            }
        }else{
            //The app has not started the MSAATester Activity.
            //This activity closes itself
            Intent i=new Intent();
            i.setClass(mContext,Activity_TestMSAA.class);
            startActivity(i);
            return;
        }
        if(FBRTesterStart){
            if(FBRTesterStop){
                //MSAA has been sucesfully tested
            }else{
                Activity_TestFB.OnCrash(this);
            }
        }else{
            //The app has not started the FBRTester Activity.
            //This activity closes itself
            Intent i=new Intent();
            i.setClass(mContext,Activity_TestFB.class);
            startActivity(i);
            return;
        }
        SharedPreferences.Editor editor=pref_static.edit();
        editor.putBoolean(getString(R.string.FirstStart),false);
        editor.commit();
        makeSuccessDialog("CPU/GPU abilities successfully tested");

    }

    private void makeStartDialog(final String text){
        final String title="First start";
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setCancelable(false);
                builder.setMessage(text)
                        .setTitle(title);
                builder.setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialogToTheBeginningHasBeenShown =true;
                        onResume();
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }

    private void makeSuccessDialog(final String text){
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setCancelable(false);
                builder.setMessage(text);
                builder.setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        makeRecommendedSettingsDialog();
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }

    private void makeRecommendedSettingsDialog(){
        SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
        boolean SingleBufferedSurfaceCreatable=pref_static.getBoolean(getString(R.string.SingleBufferedSurfaceCreatable),false);
        boolean FBAutoRefreshEnableable=pref_static.getBoolean(getString(R.string.EGL_ANDROID_front_buffer_auto_refreshAvailable),false);
        boolean EGL_KHR_reusable_syncAvailable=pref_static.getBoolean(getString(R.string.EGL_KHR_reusable_syncAvailable),false);
        boolean GL_QCOM_tiled_renderingAvailable=pref_static.getBoolean(getString(R.string.GL_QCOM_tiled_renderingAvailable),false);

        boolean sustainedPerformanceAvailable=false;
        if (Build.VERSION.SDK_INT >= 24) {
            final PowerManager powerManager = (PowerManager)getSystemService(POWER_SERVICE);
            if(powerManager!=null){
                if (powerManager.isSustainedPerformanceModeSupported()) {
                    sustainedPerformanceAvailable=true;
                }
            }
        }

        String s;
        if(SingleBufferedSurfaceCreatable&&FBAutoRefreshEnableable&&EGL_KHR_reusable_syncAvailable
                &&GL_QCOM_tiled_renderingAvailable && sustainedPerformanceAvailable){
            s="Your recommended rendering technique to reduce latency: SuperSync. You can enable SuperSync" +
                    " in 'GeneralSettings'->'Stereo/VR rendering Hacks'";

        }else if(SingleBufferedSurfaceCreatable && EGL_KHR_reusable_syncAvailable){
            s="Your recommended rendering technique to reduce latency: DisableVSYNC. You can disable VSYNC" +
                    "in 'GeneralSettings'->'Stereo/VR rendering Hacks'";
        }else{
            s="Your smartphone does not support SuperSync or DisableVSYNC. You can try if 'Disable 60 OpenGL fps cap'"+
                    "in 'GeneralSettings'->'Stereo/VR rendering Hacks' reduces latency";
        }
        final String text=s;
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setCancelable(false);
                builder.setMessage(text);
                builder.setPositiveButton("Okay", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        finish();
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }
}
