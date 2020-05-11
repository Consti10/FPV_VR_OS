package constantin.test;

import android.hardware.usb.UsbDevice;
import android.util.Log;

import java.util.HashMap;
import java.util.Iterator;

public class UVCHelper {
    public static final int DEVICE_CLASS_ROTG02=255;
    public static final int DEVICE_SUBCLASS_ROTG02=2;

    private static final String TAG="UVCHelper";
    public static HashMap<String, UsbDevice> filterFOrUVC(final HashMap<String,UsbDevice> deviceList){
        final HashMap<String,UsbDevice> ret=new HashMap<>();
        final Iterator<String> keyIterator = deviceList.keySet().iterator();
        while(keyIterator.hasNext()){
            final String key=keyIterator.next();
            final UsbDevice device = deviceList.get(key);
            if(device.getDeviceClass()==DEVICE_CLASS_ROTG02 && device.getDeviceSubclass()==DEVICE_SUBCLASS_ROTG02){
                Log.d(TAG,"Found okay device");
            }else{
                Log.d(TAG,"Not UVC cl"+device.getDeviceClass()+" subcl "+device.getDeviceSubclass());
            }
            ret.put(key,device);
        }
        return ret;
    }
}
