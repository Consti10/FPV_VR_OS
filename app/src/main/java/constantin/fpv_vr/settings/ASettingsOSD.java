package constantin.fpv_vr.settings;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
//import android.preference.PreferenceFragment;
//import android.preference.PreferenceManager;

import androidx.appcompat.app.AppCompatActivity;

import constantin.fpv_vr.R;
import constantin.fpv_vr.Toaster;

import androidx.preference.EditTextPreference;
import androidx.preference.PreferenceFragment;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;

public class ASettingsOSD extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content, new FSettingsOSD())
                .commit();
    }

    @Override
    protected void onPause(){
        super.onPause();
    }

    public static class FSettingsOSD extends PreferenceFragmentCompat {

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            //setPreferencesFromResource(R.xml.pref_lol, rootKey);
            PreferenceManager preferenceManager=getPreferenceManager();
            preferenceManager.setSharedPreferencesName("pref_osd");
            addPreferencesFromResource(R.xml.pref_osd);
        }
        @Override
        public void onActivityCreated(Bundle savedInstanceState){
            super.onActivityCreated(savedInstanceState);
        }
    }
}
