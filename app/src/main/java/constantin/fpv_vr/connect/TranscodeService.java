package constantin.fpv_vr.connect;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.util.Log;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.FutureTask;
import java.util.concurrent.ThreadPoolExecutor;

import constantin.fpv_vr.R;
import constantin.test.SimpleEncoder;

public class TranscodeService extends Service {
    private static final String TAG="TranscodeService";
    public static final String EXTRA_START_TRANSCODING_FILE="EXTRA_START_TRANSCODING_FILE";
    //public static final String EXTRA_STOP_TRANSCODING_FILE="EXTRA_STOP_TRANSCODING_FILE";

    public static final String NOTIFICATION_CHANNEL_ID = "TranscodeServiceChannel";
    public static final int NOTIFICATION_ID=1;
    private final ArrayList<Thread> workerThreads=new ArrayList<>();


    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        startForeground(NOTIFICATION_ID,createNotification());
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String startTranscoding = intent.getStringExtra(EXTRA_START_TRANSCODING_FILE);
        //String stopTranscoding=intent.getStringExtra(EXTRA_STOP_TRANSCODING_FILE);
        //System.out.println("Extras: "+startTranscoding+" "+stopTranscoding);
        System.out.println("Extras "+startTranscoding);

        final Handler handler=new Handler(getMainLooper());
        final int index=workerThreads.size();
        final Thread worker=new Thread(new Runnable() {
            @Override
            public void run() {
                new SimpleEncoder(startTranscoding).run();
                final Thread thisWorker=Thread.currentThread();
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        final boolean contained=workerThreads.remove(thisWorker);
                        System.out.println("Contained worker thread ?:"+contained);
                        updateNotification();
                        if(workerThreads.isEmpty()){
                            stopSelf();
                        }
                    }
                });
            }
        });
        workerThreads.add(worker);
        worker.start();

        updateNotification();
        //Intent notificationIntent = new Intent(this, AMain.class);
        //PendingIntent pendingIntent = PendingIntent.getActivity(this,
        //        0, notificationIntent, 0);
        //do heavy work on a background thread
        //p= SimpleTranscoder.nativeStartConvertFile(UVCReceiverDecoder.getDirectoryToSaveDataTo());
        //stopSelf();
        return START_NOT_STICKY;
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = "FPV-VR";
            String description = "Transcoder";
            NotificationChannel serviceChannel = new NotificationChannel(NOTIFICATION_CHANNEL_ID, name,
                    NotificationManager.IMPORTANCE_DEFAULT);
            serviceChannel.setDescription(description);
            NotificationManagerCompat.from(getApplication()).createNotificationChannel(serviceChannel);
        }
    }

    private Notification createNotification(){
        Bitmap bmp=Bitmap.createBitmap(128,128,Bitmap.Config.RGB_565);
        bmp.eraseColor(Color.BLUE);
        StringBuilder text= new StringBuilder();
        for(int i=0;i<workerThreads.size();i++){
            text.append("Worker ").append(i);
        }
        final Notification notification = new NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID)
                .setContentTitle("Transcoder Service")
                .setContentText(text.toString())
                .setSmallIcon(R.mipmap.ic_launcher)
                .setLargeIcon(bmp)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setOnlyAlertOnce(true)
                .build();
        return notification;
    }

    private void updateNotification(){
        final Notification notification=createNotification();
        NotificationManagerCompat.from(getApplication()).notify(NOTIFICATION_ID,notification);
    }


    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }


    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG,"onDestroy1");
        for(final Thread worker:workerThreads){
            worker.interrupt();
        }
        for(final Thread worker:workerThreads){
            try{
                worker.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        Log.d(TAG,"onDestroy2");
        NotificationManagerCompat.from(getApplication()).deleteNotificationChannel(NOTIFICATION_CHANNEL_ID);
        //NotificationManagerCompat.from(getApplication()).cancel(NOTIFICATION_CHANNEL_ID,NOTIFICATION_ID);
    }

}
