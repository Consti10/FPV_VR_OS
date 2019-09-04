package constantin.fpv_vr.rtsplib;

import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;

import constantin.fpv_vr.rtsplib.RTCPpacket;

public class RTCPSender extends Thread{
    private static final String TAG ="RTCPSender" ;
    private final int RTCP_UDP_SERVER_PORT;
    private final String SERVER_IP;

    private DatagramSocket socket;

    RTCPSender(final int rtcpPort,final String ip){
        RTCP_UDP_SERVER_PORT=rtcpPort;
        SERVER_IP=ip;
    }

    @Override
    public void run(){
        try {
            final int RTCP_UDP_CLIENT_PORT = 5601;
            socket=new DatagramSocket(RTCP_UDP_CLIENT_PORT);
            socket.setSoTimeout(100);
            byte[] bufferSend = new byte[1024];
            byte[] bufferReceive = new byte[1024];
            DatagramPacket packetReceive = new DatagramPacket(bufferReceive, bufferReceive.length);

            while(!isInterrupted()){
                {
                    RTCPpacket rtcPpacket=new RTCPpacket(0,0,0);
                    int len=rtcPpacket.getpacket(bufferSend);
                    DatagramPacket packetSend = new DatagramPacket(bufferSend, len, new InetSocketAddress(
                            SERVER_IP, RTCP_UDP_SERVER_PORT));
                    socket.send(packetSend);
                    Log.d(TAG,"Send to "+RTCP_UDP_SERVER_PORT+":"+rtcPpacket.toString());
                }
                    /*{
                        try{
                            socket.receive(packetReceive);
                            if(packetReceive.getLength()>0){
                                RTCPpacket rtcpPacket1=new RTCPpacket(packetReceive.getData(),packetReceive.getData().length);
                                Log.d(TAG,"Receive:"+rtcpPacket1.toString());
                            }else{
                                Log.d(TAG,"nothing received");
                            }
                        }catch(SocketTimeoutException e){
                            Log.d(TAG,"nothing received");
                        }
                    }*/
                sleep(1000);
            }
        }catch (InterruptedException ignored){
        }
        catch (IOException e) {
            e.printStackTrace();
        }
        if(socket!=null){
            socket.close();
        }
    }
}

    /*private RTSPResponse receiveServerResponse2() throws IOException{
        Log.d(TAG,"receiveServerResponse2() start");
        ArrayList<String> message=new ArrayList<>();
        ArrayList<String> messageBody=new ArrayList<>();
        final InputStream inputStream=mClientSocket.getInputStream();
        byte[] data=new byte[2*];
        long time=System.currentTimeMillis();
        int len;//=inputStream.read(data);
        StringBuilder s= new StringBuilder();//=new String(data,0,len);
        boolean crlfFound;
        System.out.println("CRLF l:"+CRLF.length());
        while (System.currentTimeMillis()-time<5000){
            len=inputStream.read(data);
            s.append(new String(data, 0, len));
            if(s.length()>=CRLF.length() && s.substring(s.length()-CRLF.length()).equals(CRLF)){
                System.out.println(s);
                s = new StringBuilder();
            }
        }

        while (inputStream.available()>0){
            len=inputStream.read(data);
            s.append(new String(data, 0, len));
        }
        Log.d(TAG, s.toString());
        RTSPResponse response=new RTSPResponse(message,messageBody);
        Log.d(TAG,"receiveServerResponse2() stop");
        return response;
    }*/