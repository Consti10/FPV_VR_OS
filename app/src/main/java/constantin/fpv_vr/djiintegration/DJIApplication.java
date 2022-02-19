package constantin.fpv_vr.djiintegration;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;


import java.util.concurrent.atomic.AtomicBoolean;

import constantin.fpv_vr.settings.SJ;

/**
 * If not enabled (connection type != DJI ) behaviour is like a default Android Application
 */
public class DJIApplication extends Application {
    private static final String TAG=DJIApplication.class.getSimpleName();
    private final AtomicBoolean isRegistrationInProgress = new AtomicBoolean(false);
    private long lastTimeToastDownloadDatabase=0;

    @Override
    protected void attachBaseContext(Context paramContext) {
        super.attachBaseContext(paramContext);
        Log.d(TAG,"DJI install start");
        Log.d(TAG,"DJI install stop()");
    }

    public static boolean isDJIEnabled(final Context context){
        return SJ.getConnectionType(context)==5;
    }

    public synchronized void initializeDJIIfNeeded(){

    }

    public static synchronized boolean isAircraftConnected(){
        return false;
    }


    private void debug(final String message){
        Log.d(TAG,message);
    }

    private void showToast(final String toastMsg) {
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(getBaseContext(), toastMsg, Toast.LENGTH_LONG).show();
            }
        });
    }
}