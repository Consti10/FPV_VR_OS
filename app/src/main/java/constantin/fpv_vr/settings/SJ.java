package constantin.fpv_vr.settings;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;

import constantin.fpv_vr.R;

import static android.content.Context.MODE_PRIVATE;
import static constantin.fpv_vr.connect.AConnect.CONNECTION_TYPE_TestFile;

@SuppressWarnings("WeakerAccess")

/* *
 * SJ stands for SettingsJava. Since the name of the class is written so often anywhere in my code i decided
 * to go with this shorter, but almost meaningless name
 * SJ is only a wrapper around the android shared preferences that has functions for most commonly used values
 */

public class SJ {

    //********************************** pref_connect only **********************************
    public static int getConnectionType(final Context context){
        final SharedPreferences pref_connect=context.getSharedPreferences("pref_connect", MODE_PRIVATE);
        return pref_connect.getInt(context.getString(R.string.CONNECTION_TYPE),CONNECTION_TYPE_TestFile);
    }
    @SuppressLint("ApplySharedPref")
    public static void setConnectionType(final Context context, final int value){
        final SharedPreferences pref_connect=context.getSharedPreferences("pref_connect", MODE_PRIVATE);
        pref_connect.edit().putInt(context.getString(R.string.CONNECTION_TYPE),value).commit();
    }
    //********************************** pref_connect only **********************************

    //******************************** pref_vr **************************************
    public static boolean SuperSync(final Context context){
        final SharedPreferences pref_vr= context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE);
        return pref_vr.getBoolean(context.getString(R.string.SuperSync), false);
    }

    public static boolean DisableVSYNC(final Context context) {
        final SharedPreferences pref_vr= context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE);
        return pref_vr.getBoolean(context.getString(R.string.DisableVSYNC), false);
    }

    public static boolean Disable60FPSLock(final Context context) {
        final SharedPreferences pref_vr= context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE);
        return pref_vr.getBoolean(context.getString(R.string.Disable60FPSLock), false);
    }

    public static int MultiSampleAntiAliasing(final Context context) {
        final SharedPreferences pref_vr= context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE);
        return pref_vr.getInt(context.getString(R.string.MultiSampleAntiAliasing),0);
    }

    public static boolean EnableAHT(final Context context){
        final SharedPreferences pref_vr= context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE);
        return (pref_vr.getInt(context.getString(R.string.AirHeadTrackingMode),0)!=0);
    }

    public static int AHTRefreshRateMs(final Context context){
        final SharedPreferences pref_vr = context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE);
        return pref_vr.getInt(context.getString(R.string.AHTRefreshRateMs),100);
    }

    public static int AHTPort(final Context context) {
        final SharedPreferences pref_connect = context.getSharedPreferences("pref_connect", MODE_PRIVATE);
        return pref_connect.getInt(context.getString(R.string.AHTPort),5200);
    }

    public static boolean ENABLE_LOW_PERSISTENCE(final Context context){
        final SharedPreferences pref_vr = context.getSharedPreferences("pref_vr_rendering",MODE_PRIVATE);
        return pref_vr.getBoolean(context.getString(R.string.ENABLE_LOW_PERSISTENCE),false);
    }
    //Developer
    public static boolean DEV_OVERRIDE_RENDERING_MODE_CHECK(final Context context,final SharedPreferences pref_default){
        return pref_default.getBoolean(context.getString(R.string.DEV_OVERRIDE_RENDERING_MODE_CHECK),false);
    }
    public static boolean DEV_USE_GVR_VIDEO_TEXTURE(final Context context){
        final SharedPreferences pref_developer=context.getSharedPreferences("pref_developer",MODE_PRIVATE);
        return pref_developer.getBoolean(context.getString(R.string.DEV_USE_GVR_VIDEO_TEXTURE),false);
    }
    //******************************** pref_default **************************************

    // OSD
    public static boolean OSD_DISABLE_ALL_OVERLAY_ELEMENTS(final Context context){
        final SharedPreferences pref_osd_elements=context.getSharedPreferences("pref_osd_elements",MODE_PRIVATE);
        return pref_osd_elements.getBoolean(context.getString(R.string.OSD_DISABLE_ALL_OVERLAY_ELEMENTS),false);
    }
    @SuppressLint("ApplySharedPref")
    public static void OSD_DISABLE_ALL_OVERLAY_ELEMENTS(final Context context, final boolean value){
        final SharedPreferences pref_osd_elements=context.getSharedPreferences("pref_osd_elements",MODE_PRIVATE);
        pref_osd_elements.edit().putBoolean(context.getString(R.string.OSD_DISABLE_ALL_OVERLAY_ELEMENTS),value).commit();
    }


    /*        Set<String> set=new HashSet<>();
        set.add("");

        pref_connect.edit().putStringSet("KEY",set).apply();
        Set<String> set1=pref_connect.getStringSet("KEY",null);

        CheckBoxPreference cbp;

*/

}
