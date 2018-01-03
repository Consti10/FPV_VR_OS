package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.support.v7.app.AlertDialog;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

/******************************************************
 * A Instantiation implies also a "startReceiving"
 * make sure you call "stopReceiving" after each instantiation
 *****************************************************/
public class TestReceiver extends Thread{
    static {
        System.loadLibrary("TestReceiverN");
    }
    private static native void createAllReceiverN(int videoP);
    private static native void stopAndDeleteAllReceiverN();
    private static native String getStringN();
    public static native boolean anyDataReceived();

    private TextView textView;
    private Button button;
    private Context context;

    TestReceiver(Context c,TextView tv){
        this.textView=tv;
        this.context=c;
        createAllReceiverN(Settings.UDPPortVideo);
        this.start();
    }

    TestReceiver(Context c,Button bt){
        this.button=bt;
        this.context=c;
        createAllReceiverN(Settings.UDPPortVideo);
        this.start();
    }

    public void stopReceiving(){
        this.interrupt();
        try {this.join();} catch (InterruptedException e) {e.printStackTrace();}
        stopAndDeleteAllReceiverN();
    }

    public void run(){
        setName("TestReceiver TV String refresher");
        String oldS="";
        while (!isInterrupted()){
            if(textView!=null){
                final String s=getStringN();
                if(!oldS.contentEquals(s)){
                    final boolean anyDataReceived=anyDataReceived();
                    ((Activity)context).runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            synchronized (this){
                                if(textView!=null){
                                    textView.setText(s);
                                    if(anyDataReceived){
                                        textView.setTextColor(Color.GREEN);
                                    }else{
                                        textView.setTextColor(Color.RED);
                                    }
                                }
                            }
                        }
                    });
                }
                oldS=s;
            }else{
                ((Activity)context).runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        synchronized (this){
                            if(button!=null){
                                if(anyDataReceived()){
                                    button.setTextColor(Color.GREEN);
                                }else{
                                    button.setTextColor(Color.RED);
                                }
                            }
                        }
                    }
                });
            }
            //Refresh every 200ms
            try {sleep(200);} catch (InterruptedException e) {return;}
        }
    }
}
