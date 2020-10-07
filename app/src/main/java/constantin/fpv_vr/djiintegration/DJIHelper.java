package constantin.fpv_vr.djiintegration;

import android.content.Context;
import android.util.Log;

import constantin.fpv_vr.Toaster;
import dji.common.airlink.WiFiFrequencyBand;
import dji.common.camera.ResolutionAndFrameRate;
import dji.common.camera.WhiteBalance;
import dji.common.error.DJIError;
import dji.common.util.CommonCallbacks;

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
        return "Unknown error";
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

}
