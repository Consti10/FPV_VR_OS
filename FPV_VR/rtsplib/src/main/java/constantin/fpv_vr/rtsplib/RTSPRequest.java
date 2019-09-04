package constantin.fpv_vr.rtsplib;


import android.util.Log;

public final class RTSPRequest {
    private static final String TAG="RTSPRequest";
    private static final String CRLF = "\r\n";

    public static final int CLIENT_RTP_PORT =4500;
    public static final int CLIENT_RTCP_PORT=CLIENT_RTP_PORT+1;

    public static String createRequestOPTIONS(final String rtspURI,final int seqNumber){
        String msg="OPTIONS "+rtspURI+" RTSP/1.0"+ CRLF+
                "CSeq: "+seqNumber+ CRLF+
                CRLF;
        print(msg);
        return msg;
    }

    public static String createRequestDESCRIBE(final String rtspURI,final int seqNumber){
        String msg="DESCRIBE "+rtspURI+" RTSP/1.0"+ CRLF+
                "Accept: application/sdp"+ CRLF+
                "CSeq: "+seqNumber+ CRLF+
                CRLF;
        print(msg);
        return msg;
    }

    public static String createRequestSETUP(final String rtspURIFull,final int seqNumber){
        String msg="SETUP "+rtspURIFull+" RTSP/1.0"+ CRLF+
                "CSeq: "+seqNumber+CRLF+
                //"Transport: RTP/AVP/UDP;unicast;client_port=5600-5601"+ CRLF+
                "Transport: RTP/AVP;unicast;client_port="+ CLIENT_RTP_PORT +"-"+CLIENT_RTCP_PORT+CRLF+
                CRLF;
        print(msg);
        return msg;
    }

    public static String createRequestSETUP_interleaved(final String rtspURIFull,final int seqNumber){
        String msg="SETUP "+rtspURIFull+" RTSP/1.0"+ CRLF+
                "CSeq: "+seqNumber+CRLF+
                "Transport: RTP/AVP/TCP;unicast;interleaved=0-1"+CRLF+
                CRLF;
        print(msg);
        return msg;
    }

    public static String createRequestPLAY(final String rtspUriExtracted,final int seqNumber,final String sessionID){
        String msg="PLAY "+rtspUriExtracted+" RTSP/1.0"+ CRLF+
                "CSeq: "+seqNumber+CRLF+
                "Range: npt=0-"+CRLF+
                "Session: "+sessionID+ CRLF+
                CRLF;
        print(msg);
        return msg;
    }

    public static String createRequestTEARDOWN(final String rtspUriExtracted,final int seqNumber,final String sessionID){
        String msg="TEARDOWN "+rtspUriExtracted+" RTSP/1.0"+ CRLF+
                "CSeq: "+seqNumber+CRLF+
                "Session: "+sessionID+ CRLF+
                CRLF;
        print(msg);
        return msg;
    }

    public static void print(String msg){
        String s="\n------------Request-----------------\n";
        s+=msg;
        s+="------------Request-----------------\n";
        Log.d(TAG,s);
    }
}
