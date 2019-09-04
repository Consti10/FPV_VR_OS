package constantin.fpv_vr.rtsplib;


import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.Color;
import android.util.Log;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.StringTokenizer;

//import constantin.fpv_vr.helper.Toaster;


/**
 * Work flow:
 * 1) Send an OPTIONS request and check if DESCRIBE,SETUP,PLAY and PAUSE are offered by the Server
 * -> if not, error
 *
 * 2) Send a DESCRIBE request and check if a video Stream in .h264 format exists
 * -> if not, error
 *
 * 3) Send a SETUP request and check if we can create a Session
 * -> if not, error
 */

public class MySimpleRTSPClient{
    private static final String TAG="MySimpleRTSPClient";
    private static final String CRLF = "\r\n";

    private Socket mClientSocket;
    private BufferedReader RTSPBufferedReader;
    private OutputStreamWriter mRTSPOutputStreamWriter;

    private int mRTSPSegNr;           //Sequence number of RTSP messages within the session
    private String contentBaseURI;

    private RTSPUrl mURL;
    private Thread mRTSPThread;

    private TextView mStatusTV;
    private final Activity mActivityHandle;
    public static final int MODE_TEST_1=0;
    public static final int MODE_TEST_2=1;
    public static final int MODE_SETUP_1=2;
    public static final int MODE_SETUP_2=3;


    public MySimpleRTSPClient(Activity activity,TextView statusTV){
        mActivityHandle=activity;
        mStatusTV=statusTV;

    }

    public synchronized void startSafe(int mode,final TextView statusTV,final String url){
        if(mRTSPThread!=null){
            //Toaster.makeToast(mActivityHandle,"Already started",false);
            return;
        }
        mURL=RTSPUrl.parseFromURL(url);
        System.out.println(url+" = "+mURL.toString());

        mStatusTV=statusTV;
        mStatusTV.setText("");
        mStatusTV.setTextColor(Color.BLACK);
        mRTSPSegNr=0;
        Runnable run;
        switch (mode){
            case MODE_TEST_1:
                run=new Runnable() {
                    @Override
                    public void run() {
                        testConnection1();
                        mRTSPThread=null;
                    }
                };
                break;
            case MODE_TEST_2:
                run=new Runnable() {
                    @Override
                    public void run() {
                        testConnection2();
                        mRTSPThread=null;
                    }
                };
                break;
            case MODE_SETUP_1:
                run=new Runnable() {
                    @Override
                    public void run() {
                        setup1();
                        mRTSPThread=null;
                    }
                };
                break;
            case MODE_SETUP_2:
                run=new Runnable() {
                    @Override
                    public void run() {
                        setup2();
                        mRTSPThread=null;
                    }
                };
                break;
            default:
                run=new Runnable() {
                    @Override
                    public void run() {
                        mRTSPThread=null;
                    }
                };
                break;
        }
        mRTSPThread=new Thread(run);
        mRTSPThread.start();
    }



    public void printStatus(final String text, final boolean error){
        if(mStatusTV==null){
            Log.d(TAG,text);
            return;
        }
        mActivityHandle.runOnUiThread(new Runnable() {
            @SuppressLint("SetTextI18n")
            @Override
            public void run() {
                mStatusTV.setText(mStatusTV.getText()+text);
                if(error){
                    mStatusTV.setTextColor(Color.RED);
                }
            }
        });
    }


    private void testConnection1(){
        printStatus("- START -\n",false);
        try {
            mClientSocket=new Socket();
            mClientSocket.connect(new InetSocketAddress(mURL.SERVER_IP, mURL.SERVER_PORT),5000);
            printStatus("TCP connection established\n",false);
            mClientSocket.setSoTimeout(5000);
            RTSPBufferedReader = new BufferedReader(new InputStreamReader(mClientSocket.getInputStream()));
            mRTSPOutputStreamWriter=new OutputStreamWriter(mClientSocket.getOutputStream());

            sendRequestOPTIONS();
            printStatus("OPTIONS request sent\n",false);
            RTSPResponse response=receiveServerResponse();
            printStatus("Received response from Server\n",false);
            if(response.checkResponseOPTIONS()){
                printStatus("Success ! Response OK\n",false);
            }else{
                printStatus("Wrong response. ERROR\n",true);
            }
        } catch (IOException e) {
            e.printStackTrace();
            printStatus("Exception caught. ERROR\n",true);
        }
        if(mClientSocket!=null){
            try {
                mClientSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        printStatus("- STOP -\n",false);
    }


    private void testConnection2(){
        printStatus("- START -\n",false);
        try {
            mClientSocket=new Socket();
            mClientSocket.connect(new InetSocketAddress(mURL.SERVER_IP, mURL.SERVER_PORT),5000);
            printStatus("TCP connection established\n",false);
            RTSPBufferedReader = new BufferedReader(new InputStreamReader(mClientSocket.getInputStream()));
            mRTSPOutputStreamWriter=new OutputStreamWriter(mClientSocket.getOutputStream());

            sendRequestDESCRIBE();
            printStatus("DESCRIBE request sent\n",false);
            RTSPResponse responseDESCRIBE=receiveServerResponse();
            printStatus("Received response from Server\n",false);
            String trackID=responseDESCRIBE.getMediaDescriptionVideo().getTrackID();
            if(trackID.equals("")){
                printStatus("Cannot find video. ERROR\n",true);
            }else{
                printStatus("Success ! Video found. ID: "+trackID+"\n",false);
            }
        } catch (IOException e) {
            e.printStackTrace();
            printStatus("Exception caught. ERROR\n",true);
        }
        if(mClientSocket!=null){
            try {
                mClientSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        printStatus("- STOP -\n",false);
    }

    private void setup1(){
        printStatus("- START -\n",false);
        try {
            mClientSocket=new Socket();
            mClientSocket.connect(new InetSocketAddress(mURL.SERVER_IP, mURL.SERVER_PORT),5000);
            printStatus("TCP connection established\n",false);
            mClientSocket.setSoTimeout(5000);
            RTSPBufferedReader = new BufferedReader(new InputStreamReader(mClientSocket.getInputStream()));
            mRTSPOutputStreamWriter=new OutputStreamWriter(mClientSocket.getOutputStream());

            sendRequestDESCRIBE();
            RTSPResponse responseDESCRIBE=receiveServerResponse();
            String trackID=responseDESCRIBE.getMediaDescriptionVideo().getTrackID();
            contentBaseURI=responseDESCRIBE.getContentBaseURI();
            //the ContentBase uri is optional !
            if(contentBaseURI.equals("")){
                contentBaseURI=createRTSPUri();
            }

            sendRequestSETUP(trackID);
            RTSPResponse response2=receiveServerResponse();
            String sessionID=response2.getSessionID();
            sendRequestPLAY(sessionID);
            receiveServerResponse();
            UDPReceiver rec1=new UDPReceiver(RTSPRequest.CLIENT_RTP_PORT,this);
            rec1.start();
            UDPReceiver rec2=new UDPReceiver(RTSPRequest.CLIENT_RTCP_PORT,this);
            rec2.start();
            try{
                Thread.sleep(10000);
            }catch (InterruptedException e){
                e.printStackTrace();
            }

            rec1.interrupt();
            rec2.interrupt();

            sendRequestTEARDOWN(sessionID);
            receiveServerResponse();

        } catch (IOException e) {
            e.printStackTrace();
            printStatus("Exception caught. ERROR\n",true);
        }
        if(mClientSocket!=null){
            try {
                mClientSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        printStatus("\n- STOP -\n",false);
    }

    private void setup2(){
        printStatus("- START -\n",false);
        try {
            mClientSocket=new Socket();
            mClientSocket.connect(new InetSocketAddress(mURL.SERVER_IP, mURL.SERVER_PORT),5000);
            printStatus("TCP connection established\n",false);
            mClientSocket.setSoTimeout(5000);
            RTSPBufferedReader = new BufferedReader(new InputStreamReader(mClientSocket.getInputStream()));
            mRTSPOutputStreamWriter=new OutputStreamWriter(mClientSocket.getOutputStream());

            sendRequestDESCRIBE();
            RTSPResponse responseDESCRIBE=receiveServerResponse();
            String trackID=responseDESCRIBE.getMediaDescriptionVideo().getTrackID();
            contentBaseURI=responseDESCRIBE.getContentBaseURI();
            //the ContentBase uri is optional !
            if(contentBaseURI.equals("")){
                contentBaseURI=createRTSPUri();
            }

            sendRequestSETUP_interleaved(trackID);
            RTSPResponse response2=receiveServerResponse();
            String sessionID=response2.getSessionID();

            sendRequestPLAY(sessionID);
            receiveServerResponse();

            for(int i=0;i<20;i++){
                int len=mClientSocket.getInputStream().available();
                printStatus("Data:"+len+" | ",false);
                mClientSocket.getInputStream().skip(len);
                //System.out.println("av:"+mClientSocket.getInputStream().available());
                try{
                    Thread.sleep(500);
                }catch (InterruptedException e){
                    e.printStackTrace();
                }
            }

            sendRequestTEARDOWN(sessionID);
            receiveServerResponse();

        } catch (IOException e) {
            e.printStackTrace();
            printStatus("Exception caught. ERROR\n",true);
        }
        if(mClientSocket!=null){
            try {
                mClientSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        printStatus("\n- STOP -\n",false);
    }


    private void sendMessageToServer(String msg){
        try{
            mRTSPOutputStreamWriter.write(msg);
            mRTSPOutputStreamWriter.flush();
        }catch (IOException e){
            e.printStackTrace();
        }
    }

    private String createRTSPUri(){
        return "rtsp://"+ mURL.SERVER_IP +":"+ mURL.SERVER_PORT + mURL.PATH;
    }


    private RTSPResponse receiveServerResponse() throws IOException {
        //Log.d(TAG,"receiveServerResponse() start");
        ArrayList<String> message=new ArrayList<>();
        ArrayList<String> messageBody=new ArrayList<>();

        //Read until we ether reach a timeout or a double CRLF
        final long timestamp=System.currentTimeMillis();
        String line = RTSPBufferedReader.readLine();
        Log.d(TAG,"RTSP Client - Received from Server:");
        if(line!=null){
            message.add(line);
        }
        while (System.currentTimeMillis()-timestamp<5000){
            line = RTSPBufferedReader.readLine();
            if(line!=null){
                message.add(line);
                if(line.equals("")){
                    //Log.d(TAG,"end found");
                    break;
                }
            }
        }
        int contentLength=0;
        for(String s:message){
            if((s.contains("Content-Length:"))){
                final StringTokenizer tokens = new StringTokenizer(s);
                tokens.nextToken();
                contentLength=Integer.parseInt(tokens.nextToken());
                break;
            }
        }
        int bytesRead=0;
        if(contentLength>0){
            Log.d(TAG,"Reading rest");
            try{
                line=RTSPBufferedReader.readLine();
                messageBody.add(line);
                bytesRead+=line.getBytes().length;
                while(RTSPBufferedReader.ready()){
                    line=RTSPBufferedReader.readLine();
                    messageBody.add(line);
                    bytesRead+=line.getBytes().length;
                }
            }catch (Exception e){
                //e.printStackTrace();
            }
            //Log.d(TAG,"Bytes read:"+bytesRead);
        }
        RTSPResponse response=new RTSPResponse(message,messageBody);
        response.print();
        //Log.d(TAG,"receiveServerResponse() stop");
        return response;
    }

    private void sendRequestOPTIONS(){
        Log.d(TAG,"sendRequestOptions()");
        mRTSPSegNr++;
        String rtspUri=createRTSPUri();
        String msg=RTSPRequest.createRequestOPTIONS(rtspUri,mRTSPSegNr);
        sendMessageToServer(msg);
    }

    private void sendRequestDESCRIBE(){
        Log.d(TAG,"sendRequestDescribe()");
        mRTSPSegNr++;
        String rtspUri=createRTSPUri();
        String msg=RTSPRequest.createRequestDESCRIBE(rtspUri,mRTSPSegNr);
        sendMessageToServer(msg);
    }


    private void sendRequestSETUP(final String trackID){
        Log.d(TAG,"sendRequestSetup()");
        mRTSPSegNr++;
        String rtspUriFull=createRTSPUri()+trackID;
        String msg=RTSPRequest.createRequestSETUP(rtspUriFull,mRTSPSegNr);
        sendMessageToServer(msg);
    }

    private void sendRequestSETUP_interleaved(final String trackID){
        Log.d(TAG,"sendRequestSetup_interleaved()");
        mRTSPSegNr++;
        String rtspUriFull=createRTSPUri()+trackID;
        String msg=RTSPRequest.createRequestSETUP_interleaved(rtspUriFull,mRTSPSegNr);
        sendMessageToServer(msg);
    }

    private void sendRequestPLAY(final String sessionID){
        Log.d(TAG,"sendRequestPLAY()");
        mRTSPSegNr++;
        String rtspUri=contentBaseURI;
        String msg=RTSPRequest.createRequestPLAY(rtspUri,mRTSPSegNr,sessionID);
        sendMessageToServer(msg);
    }


    private void sendRequestTEARDOWN(final String sessionID){
        Log.d(TAG,"sendRequestTEARDOWN()");
        mRTSPSegNr++;
        String rtspUri=contentBaseURI;
        String msg=RTSPRequest.createRequestTEARDOWN(rtspUri,mRTSPSegNr,sessionID);
        sendMessageToServer(msg);
    }

    private void sendDummyBytes(final int clientPort,final int serverPort){
        try {
            final DatagramSocket socket=new DatagramSocket(clientPort);
            final int dummyData=0xFEEDFACE;
            System.out.println(dummyData);
            final byte[] bytes = ByteBuffer.allocate(4).putInt(dummyData).array();
            final DatagramPacket packetSend = new DatagramPacket(bytes, bytes.length, new InetSocketAddress(
                    mURL.SERVER_IP, serverPort));
            for(int i=0;i<2;i++){
                socket.send(packetSend);
                Log.d(TAG,"Send dummy data from "+clientPort+" to "+ serverPort);
            }
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
