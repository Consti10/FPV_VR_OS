package constantin.fpv_vr.djiintegration;

import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;

import androidx.appcompat.app.AlertDialog;

import constantin.fpv_vr.Toaster;
import dji.common.airlink.WiFiFrequencyBand;
import dji.common.camera.ResolutionAndFrameRate;
import dji.common.camera.SettingsDefinitions;
import dji.common.camera.WhiteBalance;
import dji.common.error.DJIError;
import dji.common.gimbal.CapabilityKey;
import dji.common.gimbal.GimbalMode;
import dji.common.util.CommonCallbacks;
import dji.common.util.DJIParamCapability;
import dji.sdk.gimbal.Gimbal;
import dji.sdk.products.Aircraft;

public class DJIHelper {
    private static final String TAG=DJIHelper.class.getSimpleName();
    public static String frequencyBandToString(final WiFiFrequencyBand wiFiFrequencyBand){
        switch (wiFiFrequencyBand){
            case FREQUENCY_BAND_2_DOT_4_GHZ: return "2.4g";
            case FREQUENCY_BAND_5_GHZ: return "5g";
            case FREQUENCY_BAND_DUAL:return "dual";
            case FREQUENCY_BAND_ONLY_2_DOT_4: return "only 2.4";
        }
        return "unknown";
    }

    public static String asString(final DJIError djiError){
        if(djiError!=null){
            return djiError.getDescription();
        }
        return "No DJIError";
    }

    public static String asString(final ResolutionAndFrameRate[] resolutionAndFrameRate){
        StringBuilder ret= new StringBuilder();
        for(final ResolutionAndFrameRate f:resolutionAndFrameRate){
            ret.append("").append(f.toString()).append(" |");
        };
        return ret.toString();
    }

    public static String asString(final WhiteBalance whiteBalance){
        String ret="";
        ret+=whiteBalance.getWhiteBalancePreset().name();
        ret+=" CT";
        ret+=whiteBalance.getColorTemperature();
        return ret;
    }

    public static CommonCallbacks.CompletionCallback callbackLogWhenError(final Context c, final String message){
        return new CommonCallbacks.CompletionCallback() {
            @Override
            public void onResult(DJIError djiError) {
                if(djiError==null){
                    Log.d(TAG,message+" Success");
                }else{
                    Log.d(TAG,message+DJIHelper.asString(djiError));
                }
            }
        };
    }

    public static CommonCallbacks.CompletionCallback callbackToastWhenError(final Context c,final String message){
        return new CommonCallbacks.CompletionCallback() {
            @Override
            public void onResult(DJIError djiError) {
                if(djiError==null){
                    Log.d(TAG,message+" Success");
                    Toaster.makeToast(c,message+" Success");
                }else{
                    Log.d(TAG,message+DJIHelper.asString(djiError));
                    Toaster.makeToast(c,message+DJIHelper.asString(djiError));
                }
            }
        };
    }

    public static CharSequence[] getAllWhiteBalanceModes(){
        /*final CharSequence[] items = new CharSequence[7];
        for(int i=0;i<items.length;i++){
            items[i]=new SettingsDefinitions.WhiteBalancePreset(i).name();
        }*/
        return new CharSequence[]{
                "AUTO",
                "SUNNY",
                "CLOUDY",
                "WATER_SURFACE",
                "INDOOR_INCANDESCENT",
                "INDOOR_FLUORESCENT",
                "CUSTOM",
                "PRESET_NEUTRAL",
        };
    }
    public static CharSequence[] getAllGimbalModes(){
        return new CharSequence[]{
                "FREE",
                "FPV",
                "YAW_FOLLOW",
                "UNKNOWN",

        };
    }
    public static GimbalMode gimbalModeFrom(final int i){
        switch (i){
            case 0:return GimbalMode.FREE;
            case 1:return GimbalMode.FPV;
            case 2:return GimbalMode.YAW_FOLLOW;
            default:return GimbalMode.UNKNOWN;
        }
    }

    public static WhiteBalance from(final int i){
        final SettingsDefinitions.WhiteBalancePreset preset;
        switch (i){
            case 0:preset= SettingsDefinitions.WhiteBalancePreset.AUTO;break;
            case 1:preset= SettingsDefinitions.WhiteBalancePreset.SUNNY;break;
            case 2:preset= SettingsDefinitions.WhiteBalancePreset.CLOUDY;break;
            case 3:preset= SettingsDefinitions.WhiteBalancePreset.WATER_SURFACE;break;
            case 4:preset= SettingsDefinitions.WhiteBalancePreset.INDOOR_INCANDESCENT;break;
            case 5:preset= SettingsDefinitions.WhiteBalancePreset.INDOOR_FLUORESCENT;break;
            case 6:preset= SettingsDefinitions.WhiteBalancePreset.CUSTOM;break;
            case 7:preset= SettingsDefinitions.WhiteBalancePreset.PRESET_NEUTRAL;break;
            default:
                preset=  SettingsDefinitions.WhiteBalancePreset.AUTO;
        }
        return new WhiteBalance(preset);
    }

    public static AlertDialog makeAlertDialogChangeWhiteBalancePreset(final Context c){
        AlertDialog.Builder builder = new AlertDialog.Builder(c);
        builder.setTitle("Select WhiteBalance Mode")
                .setItems(DJIHelper.getAllWhiteBalanceModes(), new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        final Aircraft aircraft = DJIApplication.getConnectedAircraft();
                        if (aircraft != null) {
                            aircraft.getCamera().setWhiteBalance(DJIHelper.from(which),
                                    DJIHelper.callbackToastWhenError(c,"Set WhiteBalance"));
                        }
                    }
                });
        builder.setNegativeButton("Cancel",null);
        return builder.show();
    }

    public static AlertDialog makeAlertDialogChangeGimbalMode(final Context c){
        AlertDialog.Builder builder = new AlertDialog.Builder(c);
        builder.setTitle("Select Gimbal Mode")
                .setItems(DJIHelper.getAllGimbalModes(), new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        final Aircraft aircraft = DJIApplication.getConnectedAircraft();
                        if (aircraft != null) {
                            aircraft.getGimbal().setMode(DJIHelper.gimbalModeFrom(which),
                                    DJIHelper.callbackToastWhenError(c,"Set Gimbal FPV"));
                        }
                    }
                });
        builder.setNegativeButton("Cancel",null);
        return builder.show();
    }

    public static boolean isGimbalFeatureSupported(final Gimbal gimbal,CapabilityKey key) {
        DJIParamCapability capability = null;
        if (gimbal.getCapabilities() != null) {
            capability = gimbal.getCapabilities().get(key);
            Log.d(TAG,"Capability "+capability.toString());
        }
        if (capability != null) {
            final boolean supported=capability.isSupported();
            Log.d(TAG,"Supported "+supported);
            return supported;
        }
        Log.d(TAG,"Cannot get");
        return false;
    }

}
