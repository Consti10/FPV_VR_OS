package constantin.fpv_vr.settingsOSD;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.util.Log;
//import android.preference.PreferenceFragment;
//import android.preference.PreferenceManager;

import androidx.annotation.XmlRes;
import androidx.appcompat.app.AppCompatActivity;

import constantin.fpv_vr.R;
import constantin.fpv_vr.Toaster;
import constantin.renderingx.core.vrsettings.ASettingsVR;

import androidx.fragment.app.Fragment;
import androidx.preference.EditTextPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragment;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;

public class ASettingsOSD extends AppCompatActivity implements PreferenceFragmentCompat.OnPreferenceStartFragmentCallback{
    private static final String TAG= ASettingsOSD.class.getSimpleName();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content, new FSettingsOSD(R.xml.pref_osd))
                .commit();
    }

    @Override
    protected void onPause(){
        super.onPause();
    }

    @Override
    public boolean onPreferenceStartFragment(PreferenceFragmentCompat caller, Preference pref) {
        final Bundle args = pref.getExtras();
        //Log.d("LOL","onPreferenceStartFragment"+args.toString()+" "+pref.getFragment());
        final FSettingsOSD fragment=new FSettingsOSD(getPreferenceFileForString(pref.getFragment()));
        fragment.setArguments(args);
        fragment.setTargetFragment(caller, 0);
        // Replace the existing Fragment with the new Fragment
        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content, fragment)
                .addToBackStack(null)
                .commit();
        return true;
    }

    // The resource file containing the preferences is set in the custom constructor
    public static class FSettingsOSD extends PreferenceFragmentCompat {

        public FSettingsOSD(@XmlRes int preferencesResId){
            this.preferencesResId=preferencesResId;
        }

        private final @XmlRes int preferencesResId;

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            Log.d(TAG,"Root key"+rootKey);
            //setPreferencesFromResource(R.xml.pref_lol, rootKey);
            PreferenceManager preferenceManager=getPreferenceManager();
            Log.d(TAG,"Pref name is "+preferenceManager.getSharedPreferencesName());
            preferenceManager.setSharedPreferencesName("pref_osd");
            addPreferencesFromResource(preferencesResId);
        }

        @Override
        public void onActivityCreated(Bundle savedInstanceState){
            super.onActivityCreated(savedInstanceState);
        }
    }

    public static @XmlRes int getPreferenceFileForString(final String s){
        switch (s){
            case "pref_osd_elements":return R.xml.pref_osd_elements;
            case "pref_osd_elements1":return R.xml.pref_osd_elements1;

            case "pref_osd_style":return R.xml.pref_osd_style;
            case "pref_osd":return R.xml.pref_osd;
        }
        return 0;
    }


}
