package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

/*************
 * Writes in pref_static:
 * Stop and Start Booleans
 * MaxMSAALevel (boolean)
 * NMSAALeves  (string)
 *************/

/**********************************************
 * Prefers RGBA_8888 surfaces
 * Checks for the highest MSAA level available on the Phone (for rgba_8888 surfaces)
 * And chooses the highest MSAA level config,if available
 * else it chooses a rgba_8888 config. If even a rgba_8888 config is not available,
 * choose any config that can be used for GLSurfaceView. If even that fails, there is nothing we can choose, which will most likely result
 * in a app crash.
 * Note: Should not crash ! Made crash-safe, though
 ********************************************* */

public class Activity_TestMSAA extends AppCompatActivity {
    private static final String TAG="Activity_TestMSAA";

    public static void OnCrash(Context c){
        SharedPreferences pref_static = c.getSharedPreferences("pref_static", MODE_PRIVATE);
        SharedPreferences.Editor editor=pref_static.edit();
        editor.putString(c.getString(R.string.MaxMSAALevel),"0");
        editor.putString(c.getString(R.string.AllMSAALevels),"0#");
        editor.putBoolean(c.getString(R.string.A_MSAATester_Stop),true);
        editor.commit();
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
        SharedPreferences.Editor editor=pref_static.edit();
        editor.putBoolean(getString(R.string.A_MSAATester_Start),true);
        editor.commit();
        GLSurfaceView glSurfaceView=new GLSurfaceView(this);
        glSurfaceView.setEGLConfigChooser(new MSAAConfigChooser());
        glSurfaceView.setRenderer(new MSAARenderer());
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        setContentView(glSurfaceView);
    }

    private class MSAAConfigChooser implements GLSurfaceView.EGLConfigChooser {
        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            int attr_rgba8888_msaa[] = {
                    EGL10.EGL_LEVEL, 0, //0=default frame buffer, 1=first overlay, 2=second overlay ... Most devices only support 0
                    EGL10.EGL_RENDERABLE_TYPE, 4,//EGL_OPENGL_ES2_BIT
                    EGL10.EGL_SURFACE_TYPE,EGL10.EGL_WINDOW_BIT, //Only window suurfaces (no off-screen texture configs usw)
                    EGL10.EGL_RED_SIZE, 8,
                    EGL10.EGL_GREEN_SIZE, 8,
                    EGL10.EGL_BLUE_SIZE, 8,
                    EGL10.EGL_DEPTH_SIZE, 8,
                    EGL10.EGL_SAMPLE_BUFFERS, 1, //1 Sample Buffer for all MSAA Levels (2xMSAA,4xMSAA,...)
                    EGL10.EGL_SAMPLES,1,   //MSAA Level
                    EGL10.EGL_NONE
            };
            final int maxNConfigs=124;
            EGLConfig[] allValidConfigs = new EGLConfig[maxNConfigs];
            int[] nValidConfigs = new int[1];
            //This string will hold all MSAA levels. Integers, seperated by a '#'
            String allMSAALevels="0#";
            //At the end, the chooser will choose the highest MSAA level config, or a config without MSAA if available
            int maxMSAALevel=0;
            //Loop trough all msaa levels (i'm checking for 2xMSAA to 32xMSAA). As far as i know,
            //QCOMM GPUs support 2xMSAA and 4xMSAA while
            //MALI GPUs support 4xMSAA and 16xMSAA
            for(int msaaLevel=2;msaaLevel<=32;msaaLevel++){
                attr_rgba8888_msaa[attr_rgba8888_msaa.length-2]=msaaLevel;
                egl.eglChooseConfig(display,attr_rgba8888_msaa,allValidConfigs,maxNConfigs,nValidConfigs);
                //Log.d(TAG,"Trying to find a rgba"+msaaLevel+"xMSAA config");
                for(int j=0;j<nValidConfigs[0];j++){
                    int[] value=new int[1];
                    egl.eglGetConfigAttrib(display,allValidConfigs[j],EGL10.EGL_SAMPLES,value);
                    if(value[0]==msaaLevel){
                        Log.d(TAG,msaaLevel+"xMSAA config found");
                        allMSAALevels+=msaaLevel+"#";
                        maxMSAALevel=msaaLevel;
                        break;
                    }
                }
                //Log.d(TAG,"Cannot find a rgba "+msaaLevel+"xMSAA config");
            }
            Log.d(TAG,"Max MSAA Level:"+maxMSAALevel);
            Log.d(TAG,"All MSAA Levels:"+allMSAALevels);
            SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
            SharedPreferences.Editor editor=pref_static.edit();
            editor.putString(getString(R.string.MaxMSAALevel),""+maxMSAALevel);
            //write a string in the form "0#2#4#6..." that contains all available MSAA levels
            editor.putString(getString(R.string.AllMSAALevels),allMSAALevels);
            editor.commit();
            if(maxMSAALevel==0){
                Log.d(TAG,"Choosing a rgba_8888 config without MSAA");
                int attr_rgba8888[] = {
                        EGL10.EGL_LEVEL, 0, //0=default frame buffer, 1=first overlay, 2=second overlay ... Most devices only support 0
                        EGL10.EGL_RENDERABLE_TYPE, 4,//EGL_OPENGL_ES2_BIT
                        EGL10.EGL_SURFACE_TYPE,EGL10.EGL_WINDOW_BIT,
                        EGL10.EGL_RED_SIZE, 8,
                        EGL10.EGL_GREEN_SIZE, 8,
                        EGL10.EGL_BLUE_SIZE, 8,
                        EGL10.EGL_DEPTH_SIZE, 8,
                        EGL10.EGL_NONE
                };
                egl.eglChooseConfig(display,attr_rgba8888,allValidConfigs,maxNConfigs,nValidConfigs);
                if(nValidConfigs[0]<=0){
                    Log.w(TAG,"This phone has not any rgba_8888 config. Trying to choose anything we can render into");
                    int attr_min[] = {
                            EGL10.EGL_LEVEL, 0, //0=default frame buffer, 1=first overlay, 2=second overlay ... Most devices only support 0
                            EGL10.EGL_RENDERABLE_TYPE, 4,//EGL_OPENGL_ES2_BIT
                            EGL10.EGL_SURFACE_TYPE,EGL10.EGL_WINDOW_BIT,
                            EGL10.EGL_NONE
                    };
                    egl.eglChooseConfig(display,attr_min,allValidConfigs,maxNConfigs,nValidConfigs);
                    if(nValidConfigs[0]<=0){
                        Log.e(TAG,"There is no config available we can use for a GLSurfaceView. Something's wrong.");
                        return null;
                    }
                    return allValidConfigs[0];
                }else{
                    Log.d(TAG,"Returning a rgba_8888 config");
                }
                return allValidConfigs[0];
            }else{
                Log.d(TAG,"Choosing & returning a config with "+maxMSAALevel+"xMSAA");
                attr_rgba8888_msaa[attr_rgba8888_msaa.length-2]=maxMSAALevel;
                egl.eglChooseConfig(display,attr_rgba8888_msaa,allValidConfigs,maxNConfigs,nValidConfigs);
                return allValidConfigs[0];
            }
            //int maxMSAALevel=0;
            /*for(int msaaLevel=0;msaaLevel<=16;msaaLevel+=2){
                if(msaaLevel>0){
                    attributes_rgba8888[15]=1;
                    attributes_rgba8888[17]=msaaLevel;
                }
                System.out.println("Trying to find a "+msaaLevel+"xMSAA config");
                egl.eglChooseConfig(display, attributes_rgba8888, allConfigs, 1, numConfigs);
                int[] value=new int[1];
                egl.eglGetConfigAttrib(display,allConfigs[0],EGL10.EGL_SAMPLES,value);
                if(numConfigs[0]<=0 || value[0]!=msaaLevel){
                    System.out.println("Cannot find a "+msaaLevel+"xMSAA config."+" actual MSAALevel:"+value[0]);
                }else{
                    System.out.println(""+msaaLevel+"xMSAA config found");
                    allMSAALevels+=msaaLevel+"#";
                    maxMSAALevel=msaaLevel;
                }
            }*/


            //RGBA_8888 and usable as surface for GLSurfaceView (no off-screen render texture usw configs)
            //Since EGL_SAMPLE_BUFFERS==1 this will only return configs with ZxMSAA. Even for a 4xMSAA config you use
            //EGL_SAMPLE_Buffers 1 (with EGL_SAMPLES 4)
            /*int attr_rgba8888_msaa[] = {
                    EGL10.EGL_LEVEL, 0, //0=default frame buffer, 1=first overlay, 2=second overlay ... Most devices only support 0
                    EGL10.EGL_RENDERABLE_TYPE, 4,//EGL_OPENGL_ES2_BIT
                    EGL10.EGL_SURFACE_TYPE,EGL10.EGL_WINDOW_BIT,
                    EGL10.EGL_RED_SIZE, 8,
                    EGL10.EGL_GREEN_SIZE, 8,
                    EGL10.EGL_BLUE_SIZE, 8,
                    EGL10.EGL_DEPTH_SIZE, 8,
                    EGL10.EGL_SAMPLE_BUFFERS, 1,
                    EGL10.EGL_SAMPLES,4,
                    EGL10.EGL_NONE
            };
            //Making place for more than 512 configs resulted in a crash on a mali 450mp (some index out of bound stuff).
            //I don't think any phones have more than 512 configs in general, so this value should be sufficient btw. overkill
            final int maxNConfigs=512;
            EGLConfig[] allValidConfigs = new EGLConfig[maxNConfigs];
            int[] nValidConfigs = new int[1];
            egl.eglChooseConfig(display,attr_rgba8888_msaa,allValidConfigs,maxNConfigs,nValidConfigs);
            if(nValidConfigs[0]<=0){
                Log.w(TAG,"There are no rgba_8888 window MSAA configs on this phone. Choosing a non-MSAA config.");
                int attr_rgba8888[] = {
                        EGL10.EGL_LEVEL, 0, //0=default frame buffer, 1=first overlay, 2=second overlay ... Most devices only support 0
                        EGL10.EGL_RENDERABLE_TYPE, 4,//EGL_OPENGL_ES2_BIT
                        EGL10.EGL_SURFACE_TYPE,EGL10.EGL_WINDOW_BIT,
                        EGL10.EGL_RED_SIZE, 8,
                        EGL10.EGL_GREEN_SIZE, 8,
                        EGL10.EGL_BLUE_SIZE, 8,
                        EGL10.EGL_DEPTH_SIZE, 8,
                        EGL10.EGL_NONE
                };
                egl.eglChooseConfig(display,attr_rgba8888,allValidConfigs,maxNConfigs,nValidConfigs);
                if(nValidConfigs[0]<=0){
                    Log.e(TAG,"There are no rgba_8888 configs on this phone. Somethings' clearly wrong !");
                    return null;
                }
                return allValidConfigs[0];
            }
            Log.d(TAG,"N of valid rgba_8888 configs found on this phone:"+nValidConfigs[0]);
            //I have only heard of 2x, 4x and 16x MSAA configs on Android devices. However, just to be safe,
            //make place for all the configs that may have been returned by chooseConfig.
            int[] allMSAALevels=new int[maxNConfigs];
            for(int i=0;i<nValidConfigs[0];i++){
                int val[]=new int[1];
                egl.eglGetConfigAttrib(display,allValidConfigs[0],EGL10.EGL_SAMPLES,val);
                allMSAALevels[i]=val[0];
            }
            //multiple configs may have the same msaa level.They also might not be in ascending/descending order.
            //Sort the array in ascending order (e.g. 0xMSAA 0xMSAA 2xMSAA ...)
            Arrays.sort(allMSAALevels);
            for(int i=0;i<maxNConfigs;i++){
                System.out.println(i+"MSAA:"+allMSAALevels[i]);
            }
            //We only want to know if a ZxMSAA config exists, not how many of them
            //System.out.println("All msaa levels:"+allMSAALevels);
            //System.out.println("Max MSAA Level:"+maxMSAALevel);
            SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
            SharedPreferences.Editor editor=pref_static.edit();
            //editor.putString(mContext.getString(R.string.MaxMSAALevel),""+maxMSAALevel);
            //write a string in the form "0 2 4 6..." that contains all available MSAA levels
            //editor.putString(getString(R.string.AllMSAALevels),allMSAALevels);
            editor.commit();

            //System.out.println("Choosing and returning a "+maxMSAALevel+"xMSAA config");
            /*if(maxMSAALevel==0){
                attributes_rgba8888[15]=0;
                attributes_rgba8888[17]=0;
            }else{
                attributes_rgba8888[15]=1;
                attributes_rgba8888[17]=maxMSAALevel;
            }*/
            /*egl.eglChooseConfig(display, attr_rgba8888_msaa, allValidConfigs, 1, nValidConfigs);
            return allValidConfigs[0];*/

        }
    }

    private class MSAARenderer implements GLSurfaceView.Renderer{
        private int nFrames;
        float[] rgb=new float[3];
        @Override
        public void onSurfaceCreated(GL10 glUnused, EGLConfig config){
        }
        @Override
        public void onSurfaceChanged(GL10 glUnused, int width, int height) {
            nFrames=0;
            rgb[0]=0.0f;
            rgb[1]=0.0f;
            rgb[2]=0.0f;
        }

        @Override
        public void onDrawFrame(GL10 glUnused) {
            nFrames++;
            rgb[0]+=0.02f;
            if(rgb[0]>1){
                rgb[0]=0;
            }
            if(nFrames>60){
                SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
                SharedPreferences.Editor editor=pref_static.edit();
                editor.putBoolean(getString(R.string.A_MSAATester_Stop),true);
                editor.commit();
                finish();
            }
            GLES20.glClearColor(rgb[0],rgb[1],rgb[2],1.0f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT|GLES20.GL_DEPTH_BUFFER_BIT|GLES20.GL_STENCIL_BUFFER_BIT);
        }
    }
}
