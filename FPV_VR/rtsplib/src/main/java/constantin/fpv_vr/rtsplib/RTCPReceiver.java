package constantin.fpv_vr.rtsplib;


import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class RTCPReceiver extends Thread{
    private static final String TAG="RTCPReceiver";

    @Override
    public void run(){
        try {
            final int RTCP_UDP_PORT = 5601;
            DatagramSocket socket = new DatagramSocket(RTCP_UDP_PORT);
            byte[] buffer = new byte[1024];
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            while (!isInterrupted()){
                socket.receive(packet);
                byte[] received = packet.getData();
                Log.d(TAG,"RTCP:"+new String(received));
                //RTCPpacket rtcPpacket=new RTCPpacket(received,received.length);
                //Log.d(TAG,rtcPpacket.toString());
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}