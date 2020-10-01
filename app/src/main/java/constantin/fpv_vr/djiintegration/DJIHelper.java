package constantin.fpv_vr.djiintegration;

import dji.common.airlink.WiFiFrequencyBand;
import dji.common.camera.ResolutionAndFrameRate;
import dji.common.error.DJIError;

public class DJIHelper {

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
        String ret="";
        for(final ResolutionAndFrameRate f:resolutionAndFrameRate){
            ret+=""+f.toString()+" |";
        };
        return ret;
    }

}
