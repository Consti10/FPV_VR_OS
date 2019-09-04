package constantin.fpv_vr.rtsplib;


public class RTSPUrl {

    public final String SERVER_IP;
    public final int SERVER_PORT;
    public final String PATH;
    public final boolean valid;

    private RTSPUrl(final String host,final int port,final String path){
        SERVER_IP =host;
        SERVER_PORT =port;
        PATH =path;
        valid=true;
    }

    private RTSPUrl(){
        SERVER_IP ="";
        SERVER_PORT =0;
        PATH ="";
        valid=false;
    }

    public String toString(){
        return SERVER_IP +" "+ SERVER_PORT +" "+ PATH;
    }

    public static RTSPUrl parseFromURL(String url){
        RTSPUrl ret;
        if (url.startsWith("rtsp://")) {
            try {
                String[] data = url.split("/");
                String host = data[2].split(":")[0];
                int port = Integer.parseInt(data[2].split(":")[1]);
                StringBuilder path = new StringBuilder();
                for (int i = 3; i < data.length; i++) {
                    path.append("/").append(data[i]);
                }
                ret=new RTSPUrl(host,port, path.toString());
                //System.out.println(ret.toString());
                return ret;
            } catch (ArrayIndexOutOfBoundsException e) {
                e.printStackTrace();
            }
        }
        System.out.println("Error parsing");
        ret = new RTSPUrl();
        return ret;
    }


    public static String SkyViperURL(){
        return "rtsp://192.168.99.1:554/media/stream2";
    }
}


//try{
//            Matcher rtspMatcher = rtspUrlPattern.matcher(URL);
//            String host=rtspMatcher.group(1);
//            int port=Integer.parseInt((rtspMatcher.group(2) != null) ? rtspMatcher.group(2) : "1935");
//            String  path = "/" + rtspMatcher.group(3) + "/" + rtspMatcher.group(4);
//
//            System.out.println(host+" "+port+" "+path);
//        }catch (Exception e){
//            e.printStackTrace();
//        }

//private static final String SERVER_IP="192.168.42.129";
//private static final String SERVER_IP="192.168.99.1";
//private static final String SERVER_IP ="192.168.42.172";
//private static final String SERVER_IP="localhost";
//private static final int SERVER_PORT =554;
//private static final String PATH ="/media/stream2";
//private static final String PATH=
//private static final String PATH="ch0";