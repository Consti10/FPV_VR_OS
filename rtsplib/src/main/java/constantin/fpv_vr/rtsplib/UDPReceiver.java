package constantin.fpv_vr.rtsplib;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;

public class UDPReceiver extends Thread{
    private final int mPort;
    private final MySimpleRTSPClient instance;


    UDPReceiver(final int mPort,final MySimpleRTSPClient instance){
        this.mPort =mPort;
        this.instance=instance;
    }

    @Override
    public void run(){
        DatagramSocket socket;
        try {
            socket = new DatagramSocket(mPort);
            socket.setSoTimeout(1000);
        } catch (SocketException e) {
            e.printStackTrace();
            return;
        }
        byte[] buffer = new byte[1024*1024];
        DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
        long lastLog=System.currentTimeMillis();
        int receivedData=0;
        while (!isInterrupted()){
            try {
                socket.receive(packet);
                /*byte[] data=new byte[packet.getLength()];
                for(int i=0;i<packet.getLength();i++){
                    data[i]=packet.getData()[i];
                }*/
                receivedData+=packet.getLength();
                if(System.currentTimeMillis()-lastLog>500){
                    instance.printStatus("Data:"+receivedData+" ",false);
                    //instance.printStatus("Received "+receivedData+" B on port "+ mPort+"\n",false);
                }
                //Log.d(TAG,"Received on "+ mPort +":"+packet.getLength()+" | "+new String(data));
            } catch (IOException e) {
                instance.printStatus("Data:"+receivedData+" ",false);
                //instance.printStatus("No data on port "+mPort+" | ",false);
            }
        }
        socket.close();
    }
}