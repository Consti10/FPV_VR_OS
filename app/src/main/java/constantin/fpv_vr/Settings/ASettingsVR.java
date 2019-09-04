package constantin.fpv_vr.Settings;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.SwitchPreference;

import com.mapzen.prefsplus.IntListPreference;

import java.util.ArrayList;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import constantin.fpv_vr.R;
import constantin.renderingX.GLESInfo.GLESInfo;


public class ASettingsVR extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new FSettingsVR())
                .commit();
    }

    @Override
    protected void onPause(){
        super.onPause();
    }


    public static class FSettingsVR extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener{

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            PreferenceManager prefMgr = getPreferenceManager();
            prefMgr.setSharedPreferencesName("pref_vr_rendering");
            prefMgr.setSharedPreferencesMode(MODE_PRIVATE);
            addPreferencesFromResource(R.xml.pref_vr_rendering);
        }

        @Override
        public void onActivityCreated(Bundle savedInstanceState){
            super.onActivityCreated(savedInstanceState);
            setupMSAALevelsPreference( getActivity());
            setupHeadTrackingCategory(getPreferenceScreen().getSharedPreferences());
        }

        @Override
        public void onResume(){
            super.onResume();
            getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
            //System.out.println("name is:"+getPreferenceManager().getSharedPreferencesName());
        }

        @Override
        public void onPause(){
            super.onPause();
            getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences preferences, String key) {
            validateUserInputRenderingModes(preferences,key);
            if(key.contentEquals(getString(R.string.GroundHeadTrackingMode)) || key.contentEquals(getString(R.string.AirHeadTrackingMode))){
                setupHeadTrackingCategory(preferences);
            }
        }


        private void validateUserInputRenderingModes(SharedPreferences pref_default, String key){
            if(SJ.DEV_OVERRIDE_RENDERING_MODE_CHECK(getActivity(),pref_default)){
                return;
            }
            if(key.contentEquals(getString(R.string.DisableVSYNC))&& pref_default.getBoolean(key,false)){
                //The user tried to enable the "DisableVSYNC" option
                //This only works if there is the 'mutable' extension available, supported by ~10% today,
                //mostly on modern smartphones
                if(GLESInfo.isExtensionAvailable(getActivity(),GLESInfo.EGL_KHR_mutable_render_buffer)){
                    SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.Disable60FPSLock));
                    sp.setChecked(false);
                }else{
                    String warn="This smartphone does not support disabling VSYNC\n";
                    warn+="EGL_KHR_mutable_render_bufferAvailable: false\n";
                    makeDialog(getActivity(),warn);
                    SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.DisableVSYNC));
                    sp.setChecked(false);
                }
            }else if(key.contentEquals(getString(R.string.SuperSync)) && pref_default.getBoolean(key,false)){
                //The user enabled the "SuperSync" option
                if(!(GLESInfo.isExtensionAvailable(getActivity(),GLESInfo.EGL_KHR_mutable_render_buffer) &&
                        GLESInfo.isExtensionAvailable(getActivity(),GLESInfo.EGL_ANDROID_front_buffer_auto_refresh ))){
                    String warn="This smartphone does not support enabling SuperSync.";
                    warn+="\n-EGL_KHR_mutable_render_bufferAvailable: "+GLESInfo.isExtensionAvailable(getActivity(),GLESInfo.EGL_KHR_mutable_render_buffer);
                    warn+="\n-EGL_ANDROID_front_buffer_auto_refreshAvailable: "+GLESInfo.isExtensionAvailable(getActivity(),GLESInfo.EGL_ANDROID_front_buffer_auto_refresh );
                    warn+="\n-EGL_KHR_reusable_syncAvailable: "+GLESInfo.isExtensionAvailable(getActivity(),GLESInfo.EGL_KHR_reusable_sync );
                    makeDialog(getActivity(),warn);
                    SwitchPreference sp=(SwitchPreference)findPreference(getString(R.string.SuperSync));
                    sp.setChecked(false);
                }
            }else if(key.contentEquals(getString(R.string.Disable60FPSLock))&& pref_default.getBoolean(key,false)){
                //The user enabled the "disable60fpslock" option. Cannot be used simultaneous with "DisableVSYNC". / SuperSync
                SwitchPreference p1=(SwitchPreference)findPreference(getString(R.string.DisableVSYNC));
                p1.setChecked(false);
                SwitchPreference p2=(SwitchPreference)findPreference(getString(R.string.SuperSync));
                p2.setChecked(false);
            }
        }


        private void setupMSAALevelsPreference(final Context c){
            ArrayList<Integer> allMSAALevels=GLESInfo.availableMSAALevels(c);
            CharSequence[] msaaEntries =new CharSequence[allMSAALevels.size()];
            CharSequence[] msaaEntryValues =new CharSequence[allMSAALevels.size()];
            for(int i=0;i<allMSAALevels.size();i++){
                msaaEntries[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i)+"xMSAA";
                msaaEntryValues[allMSAALevels.size()-1-i]=""+allMSAALevels.get(i);
            }
            IntListPreference msaaPreference = (IntListPreference) findPreference(this.getString(R.string.MultiSampleAntiAliasing));
            msaaPreference.setEntries(msaaEntries);
            msaaPreference.setEntryValues(msaaEntryValues);
        }

        private void setupHeadTrackingCategory(SharedPreferences sharedPreferences){
            //head tracking. we cannot set dependencies depending on list preferences in xml, so we have to do it in java
            boolean enabledGHT=sharedPreferences.getInt(getString(R.string.GroundHeadTrackingMode),0)!=0;
            SwitchPreference sp1=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingX));
            SwitchPreference sp2=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingY));
            SwitchPreference sp3=(SwitchPreference)findPreference(getString(R.string.GroundHeadTrackingZ));
            sp1.setEnabled(enabledGHT);
            sp2.setEnabled(enabledGHT);
            sp3.setEnabled(enabledGHT);
            boolean enabledAHT=sharedPreferences.getInt(getString(R.string.AirHeadTrackingMode),0)!=0;
            SwitchPreference sp1x=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingYaw));
            SwitchPreference sp2x=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingRoll));
            SwitchPreference sp3x=(SwitchPreference)findPreference(getString(R.string.AirHeadTrackingPitch));
            sp1x.setEnabled(enabledAHT);
            sp2x.setEnabled(enabledAHT);
            sp3x.setEnabled(enabledAHT);
            ListPreference lp1=(ListPreference)findPreference(getString(R.string.AHTRefreshRateMs));
            lp1.setEnabled(enabledAHT);
        }

        private void makeDialog(final Context c, final String text){
            ((Activity)c).runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    AlertDialog.Builder builder = new AlertDialog.Builder(c);
                    builder.setCancelable(true);
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
}
