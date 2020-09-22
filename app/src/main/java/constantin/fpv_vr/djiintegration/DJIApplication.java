 package constantin.fpv_vr.djiintegration;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.multidex.MultiDex;

import com.secneo.sdk.Helper;

import java.util.concurrent.atomic.AtomicBoolean;

import dji.common.error.DJIError;
import dji.common.error.DJISDKError;
import dji.sdk.base.BaseComponent;
import dji.sdk.base.BaseProduct;
import dji.sdk.products.Aircraft;
import dji.sdk.sdkmanager.DJISDKInitEvent;
import dji.sdk.sdkmanager.DJISDKManager;

 /**
  * If not enabled (connection type != DJI ) behaviour is like a default Android Application
  */
 public class DJIApplication extends Application {
     private static final String TAG="DJIApplication";
     private final AtomicBoolean isRegistrationInProgress = new AtomicBoolean(false);
     private long lastTimeToastDownloadDatabase=0;

     @Override
     protected void attachBaseContext(Context paramContext) {
         super.attachBaseContext(paramContext);
         Log.d(TAG,"DJI install start");
         try{
             //MultiDex.install(this);
             //com.secneo.sdk.Helper.install(this);
             //initializeDJIIfNeeded();
         }catch (NoClassDefFoundError e){
             e.printStackTrace();
         }
         Log.d(TAG,"DJI install stop()");
     }

     private static int getConnectionType(final Context context){
         final SharedPreferences pref_connect=context.getSharedPreferences("pref_connect", MODE_PRIVATE);
         return pref_connect.getInt("CONNECTION_TYPE",2);
     }

     public static boolean isDJIEnabled(final Context context){
         //return SJ.getConnectionType(context)== AConnect.CONNECTION_TYPE_DJI;
         return getConnectionType(context)==5;
     }

     public static synchronized Aircraft getConnectedAircraft(){
         try{
             final BaseProduct product = DJISDKManager.getInstance().getProduct();
             if (product != null && product.isConnected() && (product instanceof Aircraft)) {
                 return (Aircraft)product;
             }
             return null;
         }catch (NoClassDefFoundError e){
             e.printStackTrace();
         }
         return null;
     }

     public static synchronized boolean isAircraftConnected(){
         return getConnectedAircraft()!=null;
     }

     public synchronized void initializeDJIIfNeeded(final AppCompatActivity parent){
         if(true){
             //return;
         }
         try{
             final Context context=getBaseContext();
             if(!isDJIEnabled(context)){
                 //return;
             }
             if(DJISDKManager.getInstance().hasSDKRegistered()){
                 return;
             }
             if(true){
                 //return;
             }
             if (isRegistrationInProgress.compareAndSet(false, true)) {
                 Log.d(TAG,"Start DJI registration");
                 AsyncTask.execute(new Runnable() {
                     @Override
                     public void run() {
                         showToast("registering, pls wait...");
                         // We mustn't override the callback directly because of the DJI install libraries process
                         DJISDKManager.getInstance().registerApp(parent, new DJISDKManager.SDKManagerCallback() {
                             @Override
                             public void onRegister(DJIError djiError) {
                                 if (djiError == DJISDKError.REGISTRATION_SUCCESS) {
                                     showToast("Register Success");
                                     //DJISDKManager.getInstance().startConnectionToProduct();
                                 } else {
                                     showToast("Register sdk fails, please check your network connection!");
                                     isRegistrationInProgress.set(false);
                                 }
                             }

                             @Override
                             public void onProductDisconnect() {
                                 showToast("Product Disconnected");
                             }

                             @Override
                             public void onProductConnect(BaseProduct baseProduct) {
                                 showToast("Product Connected");
                             }

                             @Override
                             public void onProductChanged(BaseProduct baseProduct) {

                             }

                             @Override
                             public void onComponentChange(BaseProduct.ComponentKey componentKey, BaseComponent baseComponent, BaseComponent baseComponent1) {
                                 debug("onComponentChange");
                             }

                             @Override
                             public void onInitProcess(DJISDKInitEvent djisdkInitEvent, int i) {
                                 debug("onInitProcess "+djisdkInitEvent.toString()+" "+i);
                             }

                             @Override
                             public void onDatabaseDownloadProgress(long current, long total) {
                                 final int process = (int) (100 * current / total);
                                 long delta=System.currentTimeMillis()-lastTimeToastDownloadDatabase;
                                 if(delta>5000){
                                     showToast("Downloading dji fly safe database. Make sure you have wifi connected. Process:"+process+" %");
                                     lastTimeToastDownloadDatabase=System.currentTimeMillis();
                                 }
                                 debug("onDatabaseDownloadProgress "+process+"current "+current+"total "+total);
                             }
                         });
                     }
                 });
             }
         }catch (Exception e){
             e.printStackTrace();
         }
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