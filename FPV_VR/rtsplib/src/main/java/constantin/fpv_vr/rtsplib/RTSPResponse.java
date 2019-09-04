package constantin.fpv_vr.rtsplib;

import android.util.Log;

import java.util.ArrayList;
import java.util.StringTokenizer;

public class RTSPResponse{
    private static final String TAG="RTSPResponse";
    private final ArrayList<String> message;
    private final ArrayList<String> messageBody;


    RTSPResponse(ArrayList<String> message,ArrayList<String> messageBody){
        this.message=message;
        this.messageBody = messageBody;
    }

    public final ArrayList<String> getMessage() {
        ArrayList<String> ret=new ArrayList<>();
        ret.addAll(message);
        ret.addAll(messageBody);
        return ret;
    }

    ///Returns the Session ID without the Session: part
    public String getSessionID(){
        for(String s:getMessage()){
            if(s.contains("Session:")){
                StringTokenizer tokens = new StringTokenizer(s);
                tokens.nextToken();
                String temp=tokens.nextToken(); // go to the Session ID
                if(!temp.contains(";")){
                    Log.d(TAG,"Session id (no ;): "+temp);
                    return temp;
                }
                for(int i=0;i<temp.length();i++){
                    if(temp.charAt(i)==';'){
                        temp=temp.substring(0,i);
                    }
                }
                Log.d(TAG,"Session id (with ;): "+temp);
                return temp;
            }
        }
        return "";
    }

    ///returns the content-base uri without the Content-Base part
    public String getContentBaseURI(){
        for(String s:getMessage()) {
            if (s.contains("Content-Base:")) {
                StringTokenizer tokenizer=new StringTokenizer(s);
                tokenizer.nextToken();
                String uri=tokenizer.nextToken();
                Log.d(TAG,"Content-Base uri:"+uri);
                return uri;
            }
        }
        return "";
    }

    public int getServerRTCPPort(){
        for(String s:getMessage()) {
            if (s.contains("server_port=")) {
                final int substringLen="server_port=".length();
                for(int i=0;i<s.length()-substringLen;i++){
                    if(s.substring(i,i+substringLen).equals("server_port=")){
                        final String after=s.substring(i+substringLen);
                        //loop to ether the end or come
                        for(int y=0;y<after.length();y++){
                            if(after.charAt(y)==';' || (y+1)==after.length()){
                                String ports;
                                if((y+1)==after.length()){
                                    ports=after.substring(0,y+1);
                                }else{
                                    ports=after.substring(0,y);
                                }
                                //Log.d(TAG,"PORTS:"+ports);
                                for(int z=0;z<ports.length();z++){
                                    if(ports.charAt(z)=='-'){
                                        String firstPort=ports.substring(0,z);
                                        String secondPort=ports.substring(z+1);
                                        //Log.d(TAG,firstPort+"  "+secondPort);
                                        int p1=Integer.parseInt(firstPort); // RTP port
                                        @SuppressWarnings("UnnecessaryLocalVariable") int p2=Integer.parseInt(secondPort); //RTCP port
                                        ///Log.d(TAG,"PORTS:"+p1+" "+p2);
                                        return p2;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
        return 0;
    }


    public class MediaDescriptionVideo {
        private final ArrayList<String> elements;

        MediaDescriptionVideo(){
            elements=new ArrayList<>();
        }
        MediaDescriptionVideo(ArrayList<String> elements){
            this.elements=elements;
        }
        public final ArrayList<String> getElements(){
            return elements;
        }

        ///returns the track id (e.g. xxx=0 ) with the first backslash
        public final String getTrackID(){
            for(String s:elements){
                if(s.contains("a=control:")){
                    String tmp=s.substring("a=control".length());
                    //loop from behind until we find the first '/' or ':'
                    for(int i=tmp.length()-1;i>=0;i--){
                        if(tmp.charAt(i)=='/' || tmp.charAt(i)==':'){
                            return "/"+tmp.substring(i+1);
                        }
                    }
                }
            }
            return "";
        }
    }


    ///RFC: 5.14.  Media Descriptions ("m=")
    ///A session description may contain a number of media descriptions.
    //Each media description starts with an "m=" field and is terminated by
    //either the next "m=" field or by the end of the session description.
    //A media field has several sub-fields:
    public MediaDescriptionVideo getMediaDescriptionVideo(){
        //find the 'm=video 0 RTP/AVP 96' line.
        boolean found=false;
        int counterBegin=0;
        for(String s:getMessage()){
            if(s.contains("m=video 0 RTP/AVP 96")){
                //Log.d(TAG,"Video found !");
                found=true;
                break;
            }
            counterBegin++;
        }
        if(!found){
            Log.d(TAG,"Video not found");
            return new MediaDescriptionVideo();
        }
        ArrayList<String> ret=new ArrayList<>();
        ret.add(getMessage().get(counterBegin));
        //loop until we either find a new m= part or reach the end
        for(int i=counterBegin+1;i<getMessage().size();i++){
            String s=getMessage().get(i);
            if(s.substring(0,2).equals("m=")){
                break;
            }
            ret.add(s);
        }
        return new MediaDescriptionVideo(ret);
    }

    ///check if DESCRIBE,SETUP,PLAY and TEARDOWN are offered by the Server
    public boolean checkResponseOPTIONS(){
        for(String s:getMessage()){
            if(s.contains("Public:")){
                if(s.contains("DESCRIBE") && s.contains("SETUP") && s.contains("PLAY") && s.contains("TEARDOWN")){
                    return true;
                }
            }
        }
        return false;
    }


    public void print(){
        StringBuilder string= new StringBuilder();
        string.append("\n------------Response-----------------\n");
        string.append("message:\n");
        for(String s:message){
            string.append(s).append("\n");
        }
        string.append("messageBody:\n");
        for(String s: messageBody){
            string.append(s).append("\n");
        }
        string.append("------------Response-----------------\n");
        Log.d(TAG,string.toString());
    }
}
