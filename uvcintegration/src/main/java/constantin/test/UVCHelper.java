package constantin.test;

import android.hardware.usb.UsbDevice;
import android.util.Log;

import java.util.HashMap;
import java.util.Iterator;

public class UVCHelper {
    private static final String TAG="UVCHelper";
    public static HashMap<String, UsbDevice> filterFOrUVC(final HashMap<String,UsbDevice> deviceList){
        final HashMap<String,UsbDevice> ret=new HashMap<>();
        final Iterator<String> keyIterator = deviceList.keySet().iterator();
        while(keyIterator.hasNext()){
            final String key=keyIterator.next();
            final UsbDevice device = deviceList.get(key);
            if(device.getDeviceClass()==255 && device.getDeviceSubclass()==2){
                Log.d(TAG,"Found okay device");
            }else{
                Log.d(TAG,"Not UVC cl"+device.getDeviceClass()+" subcl "+device.getDeviceSubclass());
            }
            ret.put(key,device);
        }
        return ret;
    }
}
