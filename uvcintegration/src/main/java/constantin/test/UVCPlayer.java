package constantin.test;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.SurfaceTexture;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import java.util.HashMap;
import java.util.Iterator;

import constantin.video.core.gl.ISurfaceAvailable;

// Pretty complicated / not good documented code
// Uses BroadcastReceiver to get notified when USB devices are connected / permission is granted
// Uses android lifecycle to pause / resume
// Uses SurfaceHolder.Callback to get / remove decoding surface
public class UVCPlayer extends BroadcastReceiver implements LifecycleObserver {
    private static final String TAG="UVCPlayer";
    private final UVCReceiverDecoder mUVCReceiverDecoder=new UVCReceiverDecoder();
    public static final String ACTION_USB_PERMISSION =
            "com.android.example.USB_PERMISSION";
    public static final String USB_DEVICE_ATTACHED="android.hardware.usb.action.USB_DEVICE_ATTACHED";
    public static final String USB_DEVICE_DETACHED="android.hardware.usb.action.USB_DEVICE_DETACHED";

    private final AppCompatActivity parent;
    private final UsbManager usbManager;

    public UVCPlayer(final AppCompatActivity parent){
        parent.getLifecycle().addObserver(this);
        this.parent=parent;
        usbManager=(UsbManager)parent.getSystemService(Context.USB_SERVICE);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.contentEquals(ACTION_USB_PERMISSION)) {
            Log.d(TAG,"ACTION_USB_PERMISSION");
            final UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                if(device != null){
                    final UsbDeviceConnection connection=usbManager.openDevice(device);
                    if(connection==null){
                        Log.d(TAG,"ERROR cannot get connection");
                        return;
                    }
                    mUVCReceiverDecoder.startReceiving(context,device,connection);
                }
            }
            else {
                Toast.makeText(context,"ERROR permission denied for device "+device,Toast.LENGTH_LONG).show();
            }

        }else if(action.contentEquals(USB_DEVICE_ATTACHED)){
            Log.d(TAG,"USB_DEVICE_ATTACHED");
            final UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if(device!=null){
                if(!usbManager.hasPermission(device)){
                    final PendingIntent permissionIntent = PendingIntent.getBroadcast(parent, 0, new Intent(UVCPlayer.ACTION_USB_PERMISSION), 0);
                    usbManager.requestPermission(device, permissionIntent);
                }else{
                    final UsbDeviceConnection connection=usbManager.openDevice(device);
                    if(connection==null){
                        Log.d(TAG,"ERROR cannot get connection");
                        return;
                    }
                    mUVCReceiverDecoder.startReceiving(context,device,connection);
                }
            }
        }else if(action.contentEquals(USB_DEVICE_DETACHED)){
            Log.d(TAG,"USB_DEVICE_DETACHED");
            mUVCReceiverDecoder.stopReceiving();
        }else{
            Log.d(TAG,"Unknown broadcast");
        }
    }

    private void startAlreadyConnectedUSBDevice(){
        final PendingIntent permissionIntent = PendingIntent.getBroadcast(parent, 0, new Intent(UVCPlayer.ACTION_USB_PERMISSION), 0);
        final HashMap<String, UsbDevice> deviceList =usbManager.getDeviceList();

        Log.d(TAG,"There are "+deviceList.size()+" devices connected");
        UVCHelper.filterFOrUVC(deviceList);

        final Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        while(deviceIterator.hasNext()){
            final UsbDevice device = deviceIterator.next();
            Log.d(TAG,"USB Device"+device.getDeviceName());
            usbManager.requestPermission(device, permissionIntent);
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        Log.d(TAG,"resume");
        //register the broadcast receiver
        final IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        filter.addAction(USB_DEVICE_ATTACHED);
        filter.addAction(USB_DEVICE_DETACHED);
        parent.registerReceiver(this,filter);
        // We won't get notified about already connected devices via USB_DEVICE_DETACHED broadcast
        startAlreadyConnectedUSBDevice();
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        Log.d(TAG,"pause");
        parent.unregisterReceiver(this);
        mUVCReceiverDecoder.stopReceiving();
    }

    public SurfaceHolder.Callback configure1(){
        return new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                mUVCReceiverDecoder.setSurface(holder.getSurface());
            }
            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            }
            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                mUVCReceiverDecoder.setSurface(null);
            }
        };
    }

    public ISurfaceAvailable configure2(){
        return new ISurfaceAvailable() {
            @Override
            public void XSurfaceCreated(SurfaceTexture surfaceTexture, Surface surface) {
                mUVCReceiverDecoder.setSurface(surface);
            }

            @Override
            public void XSurfaceDestroyed() {
                mUVCReceiverDecoder.setSurface(null);
            }
        };
    }

}
