package constantin.fpv_vr.Settings;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;

import androidx.appcompat.app.AppCompatActivity;
import constantin.fpv_vr.R;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.VideoPlayer.VideoSettings;

public class AGroundRecordingSettings extends AppCompatActivity {
    private static final String PREF_GROUND_RECORDING="pref_ground_recording";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final MSettingsFragment fragment=new MSettingsFragment();
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, fragment)
                .commit();
    }

    public static boolean enableHBScreenRecorder(final Context context){
        final SharedPreferences pref_ground_recording=context.getSharedPreferences(PREF_GROUND_RECORDING,MODE_PRIVATE);
        final int value=pref_ground_recording.getInt(context.getString(R.string.GROUND_RECORDING_TYPE),0);
        return value==2;
    }

    public static int getGROUND_RECORDING_VIDEO_FPS(final Context context){
        final SharedPreferences pref_ground_recording=context.getSharedPreferences(PREF_GROUND_RECORDING,MODE_PRIVATE);
        return pref_ground_recording.getInt(context.getString(R.string.GROUND_RECORDING_VIDEO_FPS),30);
    }

    public static int getGROUND_RECORDING_VIDEO_BITRATE(final Context context){
        final SharedPreferences pref_ground_recording=context.getSharedPreferences(PREF_GROUND_RECORDING,MODE_PRIVATE);
        return pref_ground_recording.getInt(context.getString(R.string.GROUND_RECORDING_VIDEO_BITRATE),40000000);
    }

    public static class MSettingsFragment extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener{

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            PreferenceManager preferenceManager=getPreferenceManager();
            preferenceManager.setSharedPreferencesName(PREF_GROUND_RECORDING);
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
                if(value==1){
                    VideoSettings.setVS_GROUND_RECORDING(getActivity(),true);
                    TelemetrySettings.setT_GROUND_RECORDING(getActivity(),true);
                }else{
                    VideoSettings.setVS_GROUND_RECORDING(getActivity(),false);
                    TelemetrySettings.setT_GROUND_RECORDING(getActivity(),false);
                }
            }
        }
    }

    private void lol(){
        Uri selectedUri = Uri.parse(Environment.getExternalStorageDirectory() + "/myFolder/");
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(selectedUri, "resource/folder");
        if (intent.resolveActivityInfo(getPackageManager(), 0) != null)
        {
            startActivity(intent);
        }
        else
        {
            // if you reach this place, it means there is no any file
            // explorer app installed on your device
        }
    }
}
