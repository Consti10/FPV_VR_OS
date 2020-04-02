package constantin.fpv_vr.Settings;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;

import androidx.appcompat.app.AppCompatActivity;
import constantin.fpv_vr.R;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.VideoPlayer.VideoSettings;

public class AGroundRecordingSettings extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final MSettingsFragment fragment=new MSettingsFragment();
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, fragment)
                .commit();
    }

    public static class MSettingsFragment extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener{

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            PreferenceManager preferenceManager=getPreferenceManager();
            preferenceManager.setSharedPreferencesName("pref_ground_recording");
            addPreferencesFromResource(R.xml.pref_ground_recording);
        }

        @Override
        public void onActivityCreated(Bundle savedInstanceState){
            super.onActivityCreated(savedInstanceState);
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
        public void onSharedPreferenceChanged(SharedPreferences pref_ground_recording, String key) {
            if(key.contentEquals(getString(R.string.GROUND_RECORDING_TYPE))){
                final int value=pref_ground_recording.getInt(key,0);
                System.out.println("HAHA");
                if(value==1){
                    System.out.println("HUHU");
                    VideoSettings.setVS_GROUND_RECORDING(getActivity(),true);
                    TelemetrySettings.setT_GROUND_RECORDING(getActivity(),true);
                }else{
                    VideoSettings.setVS_GROUND_RECORDING(getActivity(),false);
                    TelemetrySettings.setT_GROUND_RECORDING(getActivity(),false);
                }
            }
        }
    }
}
