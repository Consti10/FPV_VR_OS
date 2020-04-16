package constantin.fpv_vr.connect;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;

import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.OnLifecycleEvent;

import constantin.video.core.IsConnected;

// Adds a runnable on the activity main thread that runs
// between onPause() / onResume()
// Every REFRESH_INTERVAL_MS it checks the wifi and usb connection status
// wifi connected = wifi connected to OpenHD or
// usb connected = Tethering is enabled (in which case I also assume it is connected to OpenHD)

public class OpenHDConnectionListener implements LifecycleObserver {
    private static final int REFRESH_INTERVAL_MS=500;
    private final Handler connectionCheckHandler=new Handler();
    private final Runnable connectionCheckRunnable;
    private final Context context;
    private final IConnectionStatus iConnectionStatusChanged;

    public <T extends Activity & LifecycleOwner> OpenHDConnectionListener(final T parent, final IConnectionStatus iConnectionStatusChanged){
        this.context=parent.getApplicationContext();
        this.iConnectionStatusChanged=iConnectionStatusChanged;
        connectionCheckRunnable = new Runnable() {
            @Override
            public void run() {
                parent.runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        //check the connection statuses and change any ui elements if needed
                        updateEZWBConnectionStatus();
                    }
                });
                connectionCheckHandler.postDelayed(this, REFRESH_INTERVAL_MS);
            }
        };
        parent.getLifecycle().addObserver(this);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        connectionCheckHandler.postDelayed(connectionCheckRunnable, REFRESH_INTERVAL_MS);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        connectionCheckHandler.removeCallbacks(connectionCheckRunnable);
    }

    private void updateEZWBConnectionStatus(){
        final boolean wifiConnectedToSystem= IsConnected.checkWifiConnectedEZWB(context) ||
                IsConnected.checkWifiConnectedOpenHD(context);

        final IsConnected.USB_CONNECTION currUSBStatus=IsConnected.getUSBStatus(context);
        iConnectionStatusChanged.refreshConnectionStatus(wifiConnectedToSystem,currUSBStatus);
    }

    public interface IConnectionStatus {
        void refreshConnectionStatus(final boolean wifiConnectedToSystem, final IsConnected.USB_CONNECTION currUSBStatus);
    }
}
