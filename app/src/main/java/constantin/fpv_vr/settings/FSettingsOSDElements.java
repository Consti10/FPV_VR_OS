package constantin.fpv_vr.settings;

import android.os.Bundle;

import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;

import constantin.fpv_vr.R;

public class FSettingsOSDElements extends PreferenceFragmentCompat {

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        PreferenceManager preferenceManager=getPreferenceManager();
        preferenceManager.setSharedPreferencesName("pref_osd");
        addPreferencesFromResource(R.xml.pref_osd_elements);
    }

}