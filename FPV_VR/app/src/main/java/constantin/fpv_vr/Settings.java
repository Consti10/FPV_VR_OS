package constantin.fpv_vr;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import static android.content.Context.MODE_PRIVATE;

/**
 * Links to SettingsN
 * Variables are declared in SettingsN.h as global or constexpr variables and therefore
 * can be accessed in every cpp class without any trouble
 * In cpp they all start with a "S_", e.g. S_VR_InterpupilaryDistance
 *
 * Settings.readFromSharedPreferencesThreaded is called on each "onPause" of Activity_Settings,Activity_Connect and "onCreate" of Activity_Main.
 * Therefore, all static variables have the latest user input in any activity that is started from mainActivity, as long as
 * "waitUntilSharedPreferencesAreRead" is called before a activity is started
 */

public class Settings {
    private volatile static boolean isUpdating=false;
    private static final Object lock = new Object();

    static {
        System.loadLibrary("SettingsN");
    }
    private static native void setBoolByStringID(boolean val,String stringID);
    private static native void setFloatByStringID(float val,String stringID);
    private static native void setIntByStringID(int val,String stringID);
    private static native void setUndistortionData(float maxRSq,float k1,float k2,float k3,float k4,float k5,float k6);

    //ConnectionType/Network
    public static final int ConnectionTypeEZWB=0;
    public static final int ConnectionTypeManually=1;
    public static final int ConnectionTypeTestFile =2;
    public static final int ConnectionTypeStorageFile=3;
    public static int ConnectionType = ConnectionTypeTestFile;
    public static int UDPPortVideo=5000;
    public static String FilenameVideo="video.h264";

    //Performance "Hacks"
    public static boolean SynchronousFrontBufferRendering=false;
    public static boolean DisableVSYNC=false;
    public static boolean Disable60fpsCap =false;
    public static int MSAALevel=0; //Ranging from 0 to 16. 0=no Aliasing 16= 16xAliasing

    //HeadTracking Settings
    //public static boolean VRWorldTracking=false;
    //public static boolean CameraGimbalTracking=false;

    public static void readFromSharedPreferencesThreaded(final Context context){
        if(Settings.isUpdating){
            //Another thread is already updating
            //wait until the old thread has finished, then start a new one immediately
            try {lock.wait();} catch (InterruptedException e) {
                e.printStackTrace();
                return;
            }
        }
        synchronized (lock){
            Settings.isUpdating=true;
        }
        Thread t;
        t=new Thread() {
            @Override
            public void run() {
                synchronized (lock){
                    Settings.updateSettings(context);
                    Settings.isUpdating=false;
                    lock.notifyAll();
                }
            }
        };
        t.setName("update settings");
        t.setPriority(Thread.NORM_PRIORITY);
        t.start();
    }

    public static void waitUntilSharedPreferencesAreRead(){
        while(Settings.isUpdating){
            System.out.println("waitUntilSharedPreferencesAreRead: Not jet finished. blocking ui thread for max. 2 seconds");
            try {
                lock.wait(2000,0); //Don't wait longer than 2 seconds on the UI thread.
            } catch (IllegalMonitorStateException e) {
                //there is a small delay between the start of the update thread and when the update thread acquires the lock
                //System.out.println("Object not jet locked");
                //e.printStackTrace();
                try {Thread.sleep(1);} catch (InterruptedException e1) {e1.printStackTrace();}
            }catch (InterruptedException e){
                System.out.println("waitUntilSharedPreferencesAreRead: Updating took too long.Returning.");
                e.printStackTrace();
                return;
            }
        }
    }

    private static void updateSettings(final Context context){
        SharedPreferences pref_default= PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences pref_connect=context.getSharedPreferences("pref_connect", MODE_PRIVATE);
        ConnectionType = Integer.parseInt(pref_connect.getString(context.getString(R.string.ConnectionType),""+ ConnectionTypeTestFile));
        try{
            UDPPortVideo=Integer.parseInt(pref_connect.getString(context.getString(R.string.UDPVideoPort),"5000"));
        }catch (Exception e){e.printStackTrace();UDPPortVideo=5000;}
        FilenameVideo=pref_connect.getString(context.getString(R.string.GroundRecFileName),"video.h264");

        //DistortionCorrection
        Distortion mDistortion=new Distortion();
        float mFOV=45.0f;
        switch(pref_default.getString(context.getString(R.string.VRHeadsetType),"CV2")){
            //Cardboard Version 1
            case "CV1":mDistortion.setCoefficients(new float[]{0.441f, 0.156f});
                mFOV=40;break;
            //Cardboard Version 2
            case "CV2":mDistortion.setCoefficients(new float[]{0.34f, 0.55f});
                mFOV=60;break;
            //GearVR
            case "GVR":mDistortion.setCoefficients(new float[]{0.215f,0.215f});
                mFOV=45;break;
            //Daydream
            case "DD":mDistortion.setCoefficients(new float[]{0.42f,0.51f});
                mFOV=45;break;
            case "Manually":float k1,k2;
                mFOV=180;
                try{
                    k1 =Float.parseFloat(pref_default.getString(context.getString(R.string.K1),"0.15"));
                    k2 =Float.parseFloat(pref_default.getString(context.getString(R.string.K2),"0.15"));
                }catch (Exception e){e.printStackTrace();k1=0.15f;k2=0.15f;}
                mDistortion.setCoefficients(new float[]{k1,k2});break;
        }
        float[] undisCoef=getUndistortionCoeficients(mDistortion,mFOV);
        setUndistortionData(undisCoef[0],undisCoef[1],undisCoef[2],undisCoef[3],undisCoef[4],undisCoef[5],undisCoef[6]);

        //Performance Hacks
        SynchronousFrontBufferRendering=pref_default.getBoolean(context.getString(R.string.SuperSync), false);
        DisableVSYNC=pref_default.getBoolean(context.getString(R.string.DisableVSYNC), false);
        Disable60fpsCap =pref_default.getBoolean(context.getString(R.string.Disable60FPSLock), false);
        try{
            String string=pref_default.getString(context.getString(R.string.MultiSampleAntiAliasing),"0");
            MSAALevel=Integer.parseInt(string);
        }catch (Exception e){e.printStackTrace();MSAALevel=0;}
        //System.out.println("MSAA Level:"+MSAALevel);


        //HeadTracking
        updateIntN(pref_default,context.getString(R.string.GroundHeadTrackingMode),0);
        updateBoolN(pref_default,context.getString(R.string.GroundHeadTrackingX),true);
        updateBoolN(pref_default,context.getString(R.string.GroundHeadTrackingY),true);
        updateBoolN(pref_default,context.getString(R.string.GroundHeadTrackingZ),true);
        updateIntN(pref_default,context.getString(R.string.AirHeadTrackingMode),0);
        updateBoolN(pref_default,context.getString(R.string.AirHeadTrackingYaw),true);
        updateBoolN(pref_default,context.getString(R.string.AirHeadTrackingRoll),true);
        updateBoolN(pref_default,context.getString(R.string.AirHeadTrackingPitch),true);

//These Values are only needed in CPP.
        updateBoolN(pref_connect,context.getString(R.string.ParseLTM),false);
        updateBoolN(pref_connect,context.getString(R.string.ParseFRSKY),false);
        updateBoolN(pref_connect,context.getString(R.string.ParseMAVLINK),false);
        updateBoolN(pref_connect,context.getString(R.string.ParseRSSI),false);
        updateIntN(pref_connect,context.getString(R.string.LTMPort),5001);
        updateIntN(pref_connect,context.getString(R.string.FRSKYPort),5002);
        updateIntN(pref_connect,context.getString(R.string.MAVLINKPort),5003);
        updateIntN(pref_connect,context.getString(R.string.RSSIPort),5004);


        //VR/Stereo Settings
        updateBoolN(pref_default,context.getString(R.string.DistortionCorrection),true);
        updateFloatN(pref_default,context.getString(R.string.InterpupilaryDistance),0.2f);
        updateFloatN(pref_default,context.getString(R.string.SceneScale),50.0f);
        //OSD Values
        updateBoolN(pref_default,context.getString(R.string.TextElements),true);
        updateBoolN(pref_default,context.getString(R.string.CompassLadder),true);
        updateBoolN(pref_default,context.getString(R.string.CLHomeArrow),true);
        updateBoolN(pref_default,context.getString(R.string.CLInvertHeading),false);
        updateBoolN(pref_default,context.getString(R.string.HeightLadder),true);
        updateIntN(pref_default,context.getString(R.string.HLSelectSourceValue),0);
        updateBoolN(pref_default,context.getString(R.string.HorizonModel),true);
        updateBoolN(pref_default,context.getString(R.string.OverlaysVideo),false);
        updateBoolN(pref_default,context.getString(R.string.Roll),true);
        updateBoolN(pref_default,context.getString(R.string.Pitch),true);
        updateBoolN(pref_default,context.getString(R.string.InvertRoll),false);
        updateBoolN(pref_default,context.getString(R.string.InvertPitch),false);
        //TextElement values
        updateBoolN(pref_default,context.getString(R.string.TE_DFPS),true);
        updateBoolN(pref_default,context.getString(R.string.TE_GLFPS),true);
        updateBoolN(pref_default,context.getString(R.string.TE_TIME),true);
        updateBoolN(pref_default,context.getString(R.string.TE_RX1),true);
        updateBoolN(pref_default,context.getString(R.string.TE_RX2),true);
        updateBoolN(pref_default,context.getString(R.string.TE_RX3),true);
        updateBoolN(pref_default,context.getString(R.string.TE_BATT_P),true);
        updateBoolN(pref_default,context.getString(R.string.TE_BATT_V),true);
        updateBoolN(pref_default,context.getString(R.string.TE_BATT_A),true);
        updateBoolN(pref_default,context.getString(R.string.TE_BATT_AH),true);
        updateBoolN(pref_default,context.getString(R.string.TE_HOME_D),true);
        updateBoolN(pref_default,context.getString(R.string.TE_VS),true);
        updateBoolN(pref_default,context.getString(R.string.TE_HS),true);
        updateBoolN(pref_default,context.getString(R.string.TE_LAT),true);
        updateBoolN(pref_default,context.getString(R.string.TE_LON),true);
        updateBoolN(pref_default,context.getString(R.string.TE_HEIGHT_B),true);
        updateBoolN(pref_default,context.getString(R.string.TE_HEIGHT_GPS),true);
        updateBoolN(pref_default,context.getString(R.string.TE_N_SATS),true);
    }

    private static void updateBoolN(SharedPreferences settings,String s,boolean defVal){
        boolean val=settings.getBoolean(s, defVal);
        setBoolByStringID(val,s);
    }
    private static void updateFloatN(SharedPreferences settings,String s,float defVal){
        float floatVal;
        try{
            floatVal=Float.parseFloat(settings.getString(s,""+defVal));
        }catch (Exception e){e.printStackTrace();floatVal=1;}
        setFloatByStringID(floatVal,s);
    }
    private static void updateIntN(SharedPreferences settings,String s,int defVal){
        int intVal;
        try{
            intVal=Integer.parseInt(settings.getString(s,""+defVal));
        }catch (Exception e){e.printStackTrace();intVal=1;}
        setIntByStringID(intVal,s);
    }
    //returns _MaxRadSq and k1,k2,k3,k4,k5,k6
    private static float[] getUndistortionCoeficients(Distortion distortion,float FOV) {
        float[] results = new float[6+1];
        float maxFovHalfAngle = (float) (FOV * Math.PI / 180.0f);
        float maxRadiusLens = distortion.distortInverse(maxFovHalfAngle);
        Distortion inverse = distortion.getApproximateInverseDistortion(maxRadiusLens,6);
        results[0] = maxRadiusLens;
        results[1] = inverse.getCoefficients()[0];
        results[2] = inverse.getCoefficients()[1];
        results[3] = inverse.getCoefficients()[2];
        results[4] = inverse.getCoefficients()[3];
        results[5] = inverse.getCoefficients()[4];
        results[6] = inverse.getCoefficients()[5];
        //printM(results);
        return results;
    }

}
