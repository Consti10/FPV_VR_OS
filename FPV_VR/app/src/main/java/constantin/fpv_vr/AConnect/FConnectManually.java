package constantin.fpv_vr.AConnect;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import java.net.InetAddress;
import java.net.NetworkInterface;
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
        tv.setText(getNetworkInterfacesIPAddresses());
        mContext=getActivity();
        Button InfoB=rootView.findViewById(R.id.ManuallyInfoB);
        InfoB.setOnClickListener(this);
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        mTestReceiverVideo=new TestReceiverVideo(mContext,receivedVideoDataTV,null);
        mTestReceiverVideo.startReceiving();
        mTestReceiverTelemetry =new TestReceiverTelemetry(mContext, receivedTelemetryDataTV,null,null);
        mTestReceiverTelemetry.startReceiving();
    }

    @Override
    public void onPause() {
        super.onPause();
        mTestReceiverVideo.stopReceiving();
        mTestReceiverVideo=null;
        mTestReceiverTelemetry.stopReceiving();
        mTestReceiverTelemetry =null;
    }

    private static String getNetworkInterfacesIPAddresses(){
        StringBuilder s= new StringBuilder();
        try{
            for(Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();){
                NetworkInterface intf=en.nextElement();
                for(Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();){
                    InetAddress inetAddress=enumIpAddr.nextElement();
                    if(!intf.isLoopback() && !intf.getName().contains("dummy0")){
                        s.append("Interface ").append(intf.getName()).append(": ").append(inetAddress.getHostAddress()).append("\n");
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
