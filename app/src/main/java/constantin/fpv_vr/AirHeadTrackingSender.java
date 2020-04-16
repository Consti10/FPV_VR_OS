package constantin.fpv_vr;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.Build;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import com.google.vr.ndk.base.GvrApi;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

import constantin.fpv_vr.settings.SJ;

import static java.lang.Thread.sleep;

public class AirHeadTrackingSender implements LifecycleObserver {
    private static final String THIS_CHANNEL_ID="CHANNEL_FPV-VR_AirHeadTracking";
    private static final int THIS_NOTIFICATION_ID=1000;
    //IP and port where to  send the head tracking data
    private final InetSocketAddress mDestination;
    //Send out the ht data at a specific interval
    private final int mRefreshRateMs;
    private final Context context;
    //Use the gvrApi head tracker to get the values to send
    private final GvrApi mGvrApi;
    //When created by the first constructor, the head tracker instantiates a new GvrApi instance and handles its lifecycle
    private final boolean OwnGvrApi;
    private Thread mThread;


    public static AirHeadTrackingSender createIfEnabled(AppCompatActivity activity){
        if(SJ.EnableAHT(activity)){
            return new AirHeadTrackingSender(activity);
        }
        return null;
    };
    public static AirHeadTrackingSender createIfEnabled(AppCompatActivity activity, GvrApi gvrApi){
        if(SJ.EnableAHT(activity)){
            return new AirHeadTrackingSender(activity,gvrApi);
        }
        return null;
    };

    private AirHeadTrackingSender(AppCompatActivity activity){
        this.context=activity.getApplicationContext();
        mGvrApi=new GvrApi(activity, null);
        OwnGvrApi=true;
        mDestination=new InetSocketAddress("", SJ.AHTPort(context)); //TODO
        mRefreshRateMs = SJ.AHTRefreshRateMs(context);
        createNotificationChannel();
        activity.getLifecycle().addObserver(this);
    }

    private AirHeadTrackingSender(AppCompatActivity activity, GvrApi gvrApi){
        this.context=activity.getApplicationContext();
        OwnGvrApi=false;
        mGvrApi=gvrApi;
        mDestination=new InetSocketAddress("", SJ.AHTPort(context)); //TODO
        mRefreshRateMs = SJ.AHTRefreshRateMs(context);
        createNotificationChannel();
        activity.getLifecycle().addObserver(this);
    }

    //Start sending head tracking data
    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void startSendingDataIfEnabled(){
        if(OwnGvrApi){
            mGvrApi.resumeTracking();
        }
        String content="Sending head tracking data to "+mDestination.getAddress()+":"+mDestination.getPort()+
                " @ "+toHZ(mRefreshRateMs)+"hz"+" ("+mRefreshRateMs+"ms)";
        updateNotification(content);
        mThread=new Thread() {
            @Override
            public void run() {
                loop();
            }
        };
        mThread.start();
        System.out.println("HT STARTED");
    }

    //Stop sending head tracking data
    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void stopSendingDataIfEnabled(){
        mThread.interrupt();
        try { mThread.join(); } catch (InterruptedException ignored) { }
        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(context);
        notificationManager.cancel(THIS_NOTIFICATION_ID);
        if(OwnGvrApi){
            mGvrApi.pauseTracking();
        }
        System.out.println("HT STOPPED");
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void onDestroy(){
        if(OwnGvrApi){
            mGvrApi.shutdown();
        }
    }

    private void loop(){
        //send int16_t (2 byte) *3 (x,y,z)
        byte[] buf=new byte[3*4]; // 3 degree values Pitch_Deg, yaw and Roll_Deg, 4 bytes per float
        float[] headView=new float[16];
        DatagramSocket socket;
        DatagramPacket packet=new DatagramPacket(buf,buf.length,mDestination.getAddress(),mDestination.getPort());
        try {
            socket=new DatagramSocket();
        } catch (SocketException e) {
            e.printStackTrace();
            return;
        }
        while (!mThread.isInterrupted()){
            mGvrApi.getHeadSpaceFromStartSpaceTransform(headView,System.nanoTime() + 50000000);
            try {
                float[] degrees = getPosXYZ(headView);
                //System.out.print("HT DATA | ");
                //printMatrix(degrees);
                // Float takes 4 bytes, encoder help function
                byte[] data = encode(degrees);
                packet.setData(data);
                socket.send(packet);
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }
            try {
                sleep(mRefreshRateMs);
            } catch (InterruptedException e) {
                //A Interrupt here is no problem e.printStackTrace();
                break;
            }
        }
        System.out.println("HT STOPPED2");
        //socket.close();
    }

    private void updateNotification(String content){
        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(context);
        notificationManager.notify(THIS_NOTIFICATION_ID, buildNotification(context,content));
    }

    private void createNotificationChannel() {
        // Create the NotificationChannel, but only on API 26+ because
        // the NotificationChannel class is new and not in the support library
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = "FPV-VR";
            String description = "Air head tracking";
            int importance = NotificationManager.IMPORTANCE_DEFAULT;
            NotificationChannel channel = new NotificationChannel(THIS_CHANNEL_ID, name, importance);
            channel.setDescription(description);
            // Register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
            assert notificationManager != null;
            notificationManager.createNotificationChannel(channel);
        }
    }

    private static Notification buildNotification(Context c,String text){
        Bitmap bmp=Bitmap.createBitmap(128,128,Bitmap.Config.RGB_565);
        bmp.eraseColor(Color.BLUE);
        //Bitmap bmp=Bitmap.createScaledBitmap(BitmapFactory.decodeResource(c.getResources(),R.mipmap.ic_launcher), 128, 128, false);
        return new NotificationCompat.Builder(c,THIS_CHANNEL_ID)
                .setTicker("FPV-VR")
                .setContentTitle("Air head tracker")
                .setContentText(text)
                .setOngoing(true)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setSmallIcon(R.mipmap.ic_launcher)
                .setLargeIcon(bmp)
                .build();
    }

    public static byte[] encode (float[] floatArray) {
        byte[] byteArray = new byte[floatArray.length * 4];
        ByteBuffer byteBuf = ByteBuffer.wrap(byteArray);
        FloatBuffer floatBuf = byteBuf.asFloatBuffer();
        floatBuf.put (floatArray);
        return byteArray;
    }

    public static float[] decode (byte[] byteArray) {
        float[] floatArray = new float[byteArray.length / 4];
        ByteBuffer byteBuf = ByteBuffer.wrap(byteArray);
        FloatBuffer floatBuf = byteBuf.asFloatBuffer();
        floatBuf.get (floatArray);
        return floatArray;
    }

    private static int toHZ(int intervalMS){
        return (int)(1.0f/(intervalMS/1000.0f));
    }

    private static float[] getPosXYZ(float[] headView){
        final float[] tempM3x3=m4x4To3x3(headView);
        float[] xyz = new float[3];
        xyz[0]=(float)Math.atan2(tempM3x3[4-1],tempM3x3[0]);//R(2,1),R(1,1));
        xyz[1]=(float)Math.atan2(-tempM3x3[7-1],Math.sqrt(Math.pow(tempM3x3[8-1],2)+Math.pow(tempM3x3[9-1],2)));//-R(3,1),Math.sqrt(R(3,2)^2+R(3,3)^2)));
        xyz[2]=(float)Math.atan2(tempM3x3[8-1],tempM3x3[9-1]);//R(3,2),R(3,3));
        xyz[0]=(float)Math.toDegrees(xyz[0]);
        xyz[1]=(float)Math.toDegrees(xyz[1]);
        xyz[2]=(float)Math.toDegrees(xyz[2]);
        return xyz;
    }

    /**
     * @param mat4x4 a 4x4 matrix in the form of
     * X,X,X,0
     * X,X,X,0
     * X,X,X,0
     * 0,0,0,0
     * where only the 'X' values are valid values representing a rotation matrix in 3 axis.
     * @return a 3x3 matrix with only 'X' values
     */
    private static float[] m4x4To3x3(final float[] mat4x4){
        if(mat4x4.length!=16){
            throw new IllegalArgumentException("Not enough space to write the result");
        }
        //remove the 4th row
        float[] mat3x4=new float[3*4];
        System.arraycopy(mat4x4, 0, mat3x4, 0, 3 * 4);
        //remove the 4th column in each row
        float[] mat3x3=new float[3*3];
        int counter=0;
        for(int i=0;i<3*4;i++){
            //noinspection StatementWithEmptyBody
            if(i==4-1 || i==(4*2)-1 || i==(4*3)-1){
                //we have a 0 value (w)
            }else {
                mat3x3[counter]=mat3x4[i];
                counter++;
            }
        }
        return mat3x3;
    }

    private static void printMatrix(float[] Matrix){
        StringBuilder s= new StringBuilder();
        for (float aMatrix : Matrix) {
            s.append(aMatrix).append(";");
        }
        System.out.println(s);
    }
}