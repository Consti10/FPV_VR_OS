package constantin.fpv_vr.XDJI;

import android.app.Application;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;

import com.secneo.sdk.Helper;

import java.util.concurrent.atomic.AtomicBoolean;

import constantin.fpv_vr.AConnect.AConnect;
import constantin.fpv_vr.Settings.SJ;
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
    private final AtomicBoolean isSDKInstalled=new AtomicBoolean(false);
    private final AtomicBoolean isRegistrationInProgress = new AtomicBoolean(false);

    @Override
    protected void attachBaseContext(Context paramContext) {
        super.attachBaseContext(paramContext);
        initializeDJIIfNeeded();
    }

    public static boolean isDJIEnabled(final Context context){
        return SJ.getConnectionType(context)== AConnect.CONNECTION_TYPE_DJI;
    }

    public static Aircraft getConnectedAircraft(final Context context){
        final BaseProduct product = DJISDKManager.getInstance().getProduct();
        final Aircraft aircraft=(Aircraft)product;
        if (product == null || !product.isConnected()) {
            Toast.makeText(context,"No connected product", Toast.LENGTH_LONG).show();
        }
        return aircraft;
    }

    public void initializeDJIIfNeeded(){
        final Context context=getBaseContext();
        if(!isDJIEnabled(context)){
            return;
        }
        //This one seems to link some libraries ?
        if(isSDKInstalled.compareAndSet(false,true)){
            Helper.install(DJIApplication.this);
        }
        if(DJISDKManager.getInstance().hasSDKRegistered()){
            return;
        }
        if (isRegistrationInProgress.compareAndSet(false, true)) {
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    showToast("registering, pls wait...");
                    // We mustn't override the callback directly because of the DJI install libraries process
                    DJISDKManager.getInstance().registerApp(DJIApplication.this.getBaseContext(), new DJISDKManager.SDKManagerCallback() {
                        @Override
                        public void onRegister(DJIError djiError) {
                            if (djiError == DJISDKError.REGISTRATION_SUCCESS) {
                                showToast("Register Success");
                                DJISDKManager.getInstance().startConnectionToProduct();
                            } else {
                                showToast("Register sdk fails, please check the bundle id and network connection!");
                                isRegistrationInProgress.set(false);
                                initializeDJIIfNeeded();
                            }
                        }

                        @Override
                        public void onProductDisconnect() {

                        }

                        @Override
                        public void onProductConnect(BaseProduct baseProduct) {

                        }

                        @Override
                        public void onComponentChange(BaseProduct.ComponentKey componentKey, BaseComponent baseComponent, BaseComponent baseComponent1) {

                        }

                        @Override
                        public void onInitProcess(DJISDKInitEvent djisdkInitEvent, int i) {

                        }

                        @Override
                        public void onDatabaseDownloadProgress(long l, long l1) {

                        }
                    });
                }
            });
        }
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