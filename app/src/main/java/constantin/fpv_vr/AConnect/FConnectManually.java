package constantin.fpv_vr.AConnect;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;

import androidx.activity.ComponentActivity;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.lifecycle.LifecycleOwner;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.ArrayList;
import java.util.Enumeration;

import constantin.fpv_vr.R;
import constantin.telemetry.core.TestReceiverTelemetry;
import constantin.video.core.TestReceiverVideo;


public class FConnectManually extends Fragment implements View.OnClickListener{
    private TestReceiverTelemetry mTestReceiverTelemetry;
    private TestReceiverVideo mTestReceiverVideo;
    private TextView receivedVideoDataTV;
    private TextView receivedTelemetryDataTV;
    private Context mContext;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.connect_manually_fragment, container, false);
        receivedVideoDataTV =rootView.findViewById(R.id.FM_ReceivedVideoDataTV);
        receivedTelemetryDataTV=rootView.findViewById(R.id.FM_ReceivedTelemetryDataTV);
        TextView tv=rootView.findViewById(R.id.ipAdressesTV);
        tv.setText(getActiveInetAddresses());
        mContext=getActivity();
        FragmentActivity activity=requireActivity();
        mTestReceiverTelemetry=new TestReceiverTelemetry(activity);
        mTestReceiverTelemetry.setViews(receivedTelemetryDataTV,null,null);
        mTestReceiverVideo=new TestReceiverVideo(activity);
        mTestReceiverVideo.setViews(receivedVideoDataTV,null);
        Button InfoB=rootView.findViewById(R.id.ManuallyInfoB);
        InfoB.setOnClickListener(this);
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    //get all Inet4Addresses that are
    //either wifi or wifi hotspot or usb tethering
    private static String getActiveInetAddresses(){
        StringBuilder s= new StringBuilder();
        try{
            final Enumeration<NetworkInterface> networkInterfacesEnumeration=NetworkInterface.getNetworkInterfaces();
            while (networkInterfacesEnumeration.hasMoreElements()){
                final NetworkInterface networkInterface=networkInterfacesEnumeration.nextElement();
                if(!networkInterface.isUp() || networkInterface.getName().contains("dummy0") || networkInterface.isLoopback()){
                    continue;
                }
                final Enumeration<InetAddress> inetAddressesEnumeration=networkInterface.getInetAddresses();
                while (inetAddressesEnumeration.hasMoreElements()){
                    InetAddress inetAddress=inetAddressesEnumeration.nextElement();
                    if(inetAddress instanceof Inet4Address){
                        s.append("Interface ").append(networkInterface.getName()).append(": ").append(inetAddress.getHostAddress()).append("\n");
                    }
                }
            }
            return s.toString();
        }catch(Exception e){e.printStackTrace();}
        return "";
    }


    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.ManuallyInfoB:
                makeInfoDialog(mContext.getString(R.string.ManuallyInfo));
                break;
            default:
                break;
        }
    }

    private void makeInfoDialog(final String message){
        ((Activity)mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                builder.setMessage(message);
                AlertDialog dialog = builder.create();
                dialog.show();
            }
        });
    }
}
