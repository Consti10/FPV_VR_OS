package constantin.test;

import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import androidx.annotation.Nullable;

// Handles receiving & decoding of UVC devices that supply MJPEG frames (like ROTG02)
public class UVCReceiverDecoder {
    private static final String TAG= "";
    static{
        //System.loadLibrary("jpeg-turbo");
        //System.loadLibrary("usb1.0");
        //System.loadLibrary("uvc");
        System.loadLibrary("UVCReceiverDecoder");
    }
    private long nativeInstance;
    private boolean alreadyStreaming=false;

    public UVCReceiverDecoder(){
        nativeInstance=nativeConstruct();
    }

    public void startReceiving(final Context context,final UsbDevice device,final UsbDeviceConnection connection){
        if(alreadyStreaming){
            Log.d(TAG,"startReceiving() already called");
            return;
        }
        final String name = device.getDeviceName();
        final String[] v = !TextUtils.isEmpty(name) ? name.split("/") : null;
        int busnum = 0;
        int devnum = 0;
        if (v != null) {
            busnum = Integer.parseInt(v[v.length-2]);
            devnum = Integer.parseInt(v[v.length-1]);
        }
        //
        int success=nativeStartReceiving(nativeInstance,device.getVendorId(),device.getProductId(),connection.getFileDescriptor(),busnum,devnum,device.getDeviceName());
        if(success==0){
            alreadyStreaming=true;
        }
    }

    public void stopReceiving(){
        nativeStopReceiving(nativeInstance);
        alreadyStreaming=false;
    }

    /**
     * If @param surface!=null a native reference is created and
     * future uvc frames will be decoded into the underlying buffer(s) of the surface
     * If @param surface==null the native reference will be deleted and future frames won't be decoded
     */
    public void setSurface(@Nullable Surface surface){
        nativeSetSurface(nativeInstance,surface);
    }

    private static native long nativeConstruct();
    private static native void nativeDelete(long nativeInstance);
    // returns 0 on success
    private static native int nativeStartReceiving(long nativeInstance,int venderId, int productId, int fileDescriptor, int busNum, int devAddr, String usbfs);
    private static native void nativeStopReceiving(long nativeInstance);
    private static native void nativeSetSurface(long nativeInstance,Surface surface);



}
