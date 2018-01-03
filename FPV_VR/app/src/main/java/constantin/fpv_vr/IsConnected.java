package constantin.fpv_vr;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;


public class IsConnected {
    public static final int USB_NOTHING=0,USB_CONNECTED=1,USB_TETHERING=2;

    public static boolean checkWifiConnectedToEZWB(Context context){
        boolean M1Connected=false;
        WifiManager wifiManager = (WifiManager) context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        if (wifiInfo.getSupplicantState() == SupplicantState.COMPLETED ) {
            String ssid = wifiInfo.getSSID();
            if(ssid.equals("\"EZ-WifiBroadcast\"")||ssid.equals("\"Connectify-0\"")){
                M1Connected=true;
            }else {
                M1Connected=false;
            }
            //System.out.println("SSID:"+ssid);
        }
        return M1Connected;
        //System.out.println(wifiInfo.getSupplicantState().toString());
    }


    //0: nothing; 1:usb connected 2:usb connected&usb tethering
    public static int checkTetheringConnectedToEZWB(Context context){
        if(isUSBConnected(context)){
            if(isTetheringActive(context)){
                return USB_TETHERING;
            }else{
                return USB_CONNECTED;
            }
        }else{
            return USB_NOTHING;
        }
    }

    public static boolean isUSBConnected(Context context){
        Intent intent = context.registerReceiver(null, new IntentFilter("android.hardware.usb.action.USB_STATE"));
        return intent.getExtras().getBoolean("connected");
    }

    private static boolean isTetheringActive(Context context){
        try{
            for(Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();){
                NetworkInterface intf=en.nextElement();
                for(Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();){
                    InetAddress inetAddress=enumIpAddr.nextElement();
                    if(!intf.isLoopback()){
                        if(intf.getName().contains("rndis")){
                            return true;
                        }
                    }
                }
            }
        }catch(Exception e){e.printStackTrace();}
        return false;
    }

}
