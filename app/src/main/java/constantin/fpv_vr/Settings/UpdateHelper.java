package constantin.fpv_vr.Settings;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import constantin.fpv_vr.AConnect.AConnect;
import constantin.fpv_vr.R;
import constantin.telemetry.core.TelemetrySettings;
import constantin.video.core.VideoPlayer.VideoSettings;

import static android.content.Context.MODE_PRIVATE;

public class UpdateHelper {

    private static final String FIRST_START_30 ="FIRST_START_30";

    //set default values for all preference files of the fpv-vr library/module
    private static void setAllDefaultValues(final Context c, final boolean readAgain){
        PreferenceManager.setDefaultValues(c,"pref_developer",MODE_PRIVATE, R.xml.pref_developer,readAgain);
        PreferenceManager.setDefaultValues(c,"pref_connect",MODE_PRIVATE,R.xml.pref_connect,readAgain);
        PreferenceManager.setDefaultValues(c,"pref_vr_rendering",MODE_PRIVATE,R.xml.pref_vr_rendering,readAgain);
        PreferenceManager.setDefaultValues(c,"pref_osd",MODE_PRIVATE,R.xml.pref_osd_elements,readAgain);
        PreferenceManager.setDefaultValues(c,"pref_osd",MODE_PRIVATE,R.xml.pref_osd_style,readAgain);
        TelemetrySettings.initializePreferences(c,readAgain);
        VideoSettings.initializePreferences(c,readAgain);
        AConnect.setPreferencesForConnectionType(c,c.getSharedPreferences("pref_connect",MODE_PRIVATE).getInt(c.getString(R.string.ConnectionType),2));
    }

    @SuppressLint("ApplySharedPref")
    private static void clearPreviousPreferences(final Context c){
        final String[] preferenceNames=new String[]{
                "pref_developer","pref_connect","pref_vr_rendering","pref_osd","pref_final",
                "pref_telemetry","pref_video"
        };
        for(final String s:preferenceNames){
            c.getSharedPreferences(s,MODE_PRIVATE).edit().clear().commit();
        }
        PreferenceManager.getDefaultSharedPreferences(c).edit().clear().commit();
    }

    @SuppressLint("ApplySharedPref")
    public static void checkForFreshInstallOrUpdate(final Context c){
        final SharedPreferences pref_default= PreferenceManager.getDefaultSharedPreferences(c);
        if(pref_default.getBoolean(FIRST_START_30,true)){
            clearPreviousPreferences(c);
            setAllDefaultValues(c,true);
            pref_default.edit().putBoolean(FIRST_START_30,false).commit();
        }
    }

}
