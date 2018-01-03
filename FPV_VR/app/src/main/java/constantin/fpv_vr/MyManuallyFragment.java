package constantin.fpv_vr;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;


public class MyManuallyFragment extends Fragment {
    private TestReceiver mTestReceiver;
    private TextView ManuallyReceiveTV;
    private Context mContext;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.my_manually_fragment, container, false);
        ManuallyReceiveTV=rootView.findViewById(R.id.ManuallyReceiveTV);
        TextView tv=rootView.findViewById(R.id.ipAdressesTV);
        tv.setText(getNetworkInterfacesIPAdresses());
        mContext=getActivity();
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        //System.out.println("MF: onResume");
        mTestReceiver=new TestReceiver(mContext,ManuallyReceiveTV);
    }

    @Override
    public void onPause() {
        super.onPause();
        //System.out.println("MF: onPause");
        mTestReceiver.stopReceiving();
        mTestReceiver=null;
    }

    public static String getNetworkInterfacesIPAdresses(){
        String s="";
        try{
            for(Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();){
                NetworkInterface intf=en.nextElement();
                for(Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();){
                    InetAddress inetAddress=enumIpAddr.nextElement();
                    if(!intf.isLoopback() && !intf.getName().contains("dummy0")){
                        s+="Interface "+intf.getName()+": "+inetAddress.getHostAddress()+"\n";
                    }
                }
            }
            return s;
        }catch(Exception e){e.printStackTrace();}
        return "";
    }
}
