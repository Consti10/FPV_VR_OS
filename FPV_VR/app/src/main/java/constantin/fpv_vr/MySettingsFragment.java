package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.SwitchPreference;
import android.support.v7.app.AlertDialog;

import java.util.ArrayList;

import static android.content.Context.MODE_PRIVATE;


public class MySettingsFragment extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener{
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.preferences);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState){
        super.onActivityCreated(savedInstanceState);
        //This is for the MSAALevel
        SharedPreferences pref_static = getActivity().getSharedPreferences("pref_static", MODE_PRIVATE);
        //System.out.println(pref_static.getString(getString(R.string.MaxMSAALevel),"0"));
        String AllMSAALevels=pref_static.getString(getString(R.string.AllMSAALevels),"0#");
        ArrayList<Integer> allMSAALevels=new ArrayList<>();
        int indx=0;
        for(int i=0;i<AllMSAALevels.length();i++){
            char c=AllMSAALevels.charAt(i);
            if(c=='#'){
                String sub=AllMSAALevels.substring(indx,i);
                //System.out.println("Sbustring:"+sub);
                int msaaLevel=Integer.parseInt(sub);
                allMSAALevels.add(msaaLevel);
                indx=i+1;
            }
        }
        CharSequence[] msaaEntries =new CharSequence[allMSAALevels.size()];
        CharSequence[] msaaEntryValues =new CharSequence[allMSAALevels.size()];
        for(int i=0;i<allMSAALevels.size();i++){
            msaaEntries[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i)+"xMSAA";
            msaaEntryValues[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i);
        }
        ListPreference msaaPreference = (ListPreference) findPreference(this.getString(R.string.MultiSampleAntiAliasing));
        msaaPreference.setEntries(msaaEntries);
        msaaPreference.setEntryValues(msaaEntryValues);

        //head tracking enable/disable yaw roll pitch usw
        SharedPreferences pref_default = PreferenceManager.getDefaultSharedPreferences(getActivity());
        boolean enabled=!pref_default.getString(getString(R.string.GroundHeadTrackingMode),"0").contentEquals("0");
        SwitchPreference sp1=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingX));
        SwitchPreference sp2=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingY));
        SwitchPreference sp3=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingZ));
        sp1.setEnabled(enabled);
        sp2.setEnabled(enabled);
        sp3.setEnabled(enabled);
        enabled=!pref_default.getString(getString(R.string.AirHeadTrackingMode),"0").contentEquals("0");
        sp1=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingYaw));
        sp2=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingRoll));
        sp3=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingPitch));
        sp1.setEnabled(enabled);
        sp2.setEnabled(enabled);
        sp3.setEnabled(enabled);
    }
    @Override
    public void onResume(){
        super.onResume();
        getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onPause(){
        super.onPause();
        getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        if(key.contentEquals(getString(R.string.DisableVSYNC))&& sharedPreferences.getBoolean(key,false)){
            //The user tried to enable the "DisableVSYNC" option
            SharedPreferences pref_static = getActivity().getSharedPreferences("pref_static", MODE_PRIVATE);
            if(!pref_static.getBoolean(getString(R.string.SingleBufferedSurfaceCreatable),false)
                    || !pref_static.getBoolean(getString(R.string.EGL_ANDROID_front_buffer_auto_refreshAvailable),false)){
                //This phone does not support a single buffered surface. Cannot disable VSYNC.
                //WARNING: I think on android devices <Android nougat, isSurfaceSingleBuffered returns true though
                //it actually isn't (tested on android 6, galaxy j5 2016: glFlush has no results on the screen.)
                //Question to be resolved: are single buffered surfaces only creatable on devices
                //a) >=Android 7 or
                //b) with "Auto refresh" extension or
                //c) with "Mutable render buffer" extension.
                //03.12.2017: auto refresh is supported on ~10% (they are all android 7 devices), so i go with it on my release,
                //though phones exist that have android 7, but don't support auto refresh
                //Toaster.makeToast(getActivity().getBaseContext(),"This smartphone does not support disabling VSYNC",false);
                String warn="This phone does not support disabling VSYNC.";
                warn+="\n-SingleBufferedSurfaceCreatable: "+pref_static.getBoolean(getString(R.string.SingleBufferedSurfaceCreatable),false);
                warn+="\n-EGL_ANDROID_front_buffer_auto_refreshAvailable: "+pref_static.getBoolean(getString(R.string.EGL_ANDROID_front_buffer_auto_refreshAvailable),false);
                makeDialog(getActivity(),warn);
                SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.DisableVSYNC));
                sp.setChecked(false);
            }else{
                SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.Disable60FPSLock));
                sp.setChecked(false);
            }
        }else if(key.contentEquals(getString(R.string.Disable60FPSLock))&& sharedPreferences.getBoolean(key,false)){
            //The user enabled the "disable60fpscap" option. Cannot be used simultaneous with "DisableVSYNC".
            SwitchPreference p1=(SwitchPreference)findPreference(getString(R.string.DisableVSYNC));
            p1.setChecked(false);
        }else if(key.contentEquals(getString(R.string.SuperSync)) && sharedPreferences.getBoolean(key,false)){
            //The user enabled the "SuperSync" option
            SharedPreferences pref_static = getActivity().getSharedPreferences("pref_static", MODE_PRIVATE);
            if(!pref_static.getBoolean(getString(R.string.SingleBufferedSurfaceCreatable),false) ||
                    !pref_static.getBoolean(getString(R.string.EGL_ANDROID_front_buffer_auto_refreshAvailable),false) ||
                    !pref_static.getBoolean(getString(R.string.EGL_KHR_reusable_syncAvailable),false)){
                //This phone does not fulfill the soft requirements for SFBR
                //Toaster.makeToast(getActivity().getBaseContext(),"This smartphone does not support synchronous front buffer rendering.",false);
                String warn="This phone does not support disabling SyncFBR.";
                warn+="\n-SingleBufferedSurfaceCreatable: "+pref_static.getBoolean(getString(R.string.SingleBufferedSurfaceCreatable),false);
                warn+="\n-EGL_ANDROID_front_buffer_auto_refreshAvailable: "+pref_static.getBoolean(getString(R.string.EGL_ANDROID_front_buffer_auto_refreshAvailable),false);
                warn+="\n-EGL_KHR_reusable_syncAvailable: "+pref_static.getBoolean(getString(R.string.EGL_KHR_reusable_syncAvailable),false);
                makeDialog(getActivity(),warn);
                SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.SuperSync));
                sp.setChecked(false);
            }
        }
        //head tracking. we cannot set dependencies depending on list preferences in xml, so we have to do it in java
        if(key.contentEquals(getString(R.string.GroundHeadTrackingMode))){
            boolean enabled=!sharedPreferences.getString(key,"").contentEquals("0");
            SwitchPreference sp1=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingX));
            SwitchPreference sp2=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingY));
            SwitchPreference sp3=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingZ));
            sp1.setEnabled(enabled);
            sp2.setEnabled(enabled);
            sp3.setEnabled(enabled);
        }
        if(key.contentEquals(getString(R.string.AirHeadTrackingMode))){
            boolean enabled=!sharedPreferences.getString(key,"").contentEquals("0");
            SwitchPreference sp1=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingYaw));
            SwitchPreference sp2=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingRoll));
            SwitchPreference sp3=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingPitch));
            sp1.setEnabled(enabled);
            sp2.setEnabled(enabled);
            sp3.setEnabled(enabled);
        }
        //System.out.println("HI"+key);*/
    }

    private void makeDialog(final Context c,final String text){
        ((Activity)c).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(c);
                builder.setCancelable(false);
                builder.setMessage(text);
                builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        //just go away
                    }
                });
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }

}
