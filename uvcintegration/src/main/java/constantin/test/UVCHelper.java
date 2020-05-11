package constantin.test;

import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.util.HashMap;
import java.util.Iterator;

public class UVCHelper {
    public static final int DEVICE_CLASS_ROTG02=239;
    public static final int DEVICE_SUBCLASS_ROTG02=2;
    private static final String android_hardware_usb_action_USB_DEVICE_ATTACHED="android.hardware.usb.action.USB_DEVICE_ATTACHED";

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

    // Logs a toast to the user to change settings to UVC if
    // The activity was started via the intent-filter -> action declared in the manifest
    public static void informIfStartedViaIntentFilter(final AppCompatActivity activity){
        final Intent intent=activity.getIntent();
        if (intent != null) {
            final String action=intent.getAction();
            if(action!=null && action.contentEquals(android_hardware_usb_action_USB_DEVICE_ATTACHED)){
                Toast.makeText(activity,"Please select UVC as connection type under CONNECT",Toast.LENGTH_LONG).show();
            }
        }
    }
}
