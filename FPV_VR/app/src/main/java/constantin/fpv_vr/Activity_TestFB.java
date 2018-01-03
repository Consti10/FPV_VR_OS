package constantin.fpv_vr;

import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

/* ************************************************
 * Writes in pref_static:
 * Stop and Start booleans
 * GL_QCOM_tiled_renderingAvailable
 * EGL_KHR_reusable_syncAvailable
 * EGL_ANDROID_front_buffer_auto_refreshAvailable
 * SingleBufferedSurfaceCreatable
 ************************************************** */

public class Activity_TestFB extends AppCompatActivity {


    public static void OnCrash(Context c){
        SharedPreferences pref_static = c.getSharedPreferences("pref_static", MODE_PRIVATE);
        SharedPreferences.Editor editor=pref_static.edit();
        editor.putBoolean(c.getString(R.string.GL_QCOM_tiled_renderingAvailable),false);
        editor.putBoolean(c.getString(R.string.EGL_KHR_reusable_syncAvailable),false);
        editor.putBoolean(c.getString(R.string.EGL_ANDROID_front_buffer_auto_refreshAvailable),false);
        editor.putBoolean(c.getString(R.string.SingleBufferedSurfaceCreatable),false);
        editor.putBoolean(c.getString(R.string.A_FBTester_Stop),true);
        editor.commit();
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
        SharedPreferences.Editor editor=pref_static.edit();
        editor.putBoolean(getString(R.string.A_FBTester_Start),true);
        editor.commit();
        GLSurfaceViewEGL14 mGLView14_FBTest = new GLSurfaceViewEGL14(this, GLSurfaceViewEGL14.MODE_SYNCHRONOUS_FRONT_BUFFER_RENDERING, 0);
        GLRenderer14FBTest mTestRenderer = new GLRenderer14FBTest();
        mGLView14_FBTest.setRenderer(mTestRenderer);
        mGLView14_FBTest.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLView14_FBTest.setPreserveEGLContextOnPause(false);
        setContentView(mGLView14_FBTest);
    }


    private class GLRenderer14FBTest implements GLSurfaceViewEGL14.IRendererEGL14{
        int nFrames;
        float[] rgb=new float[3];

        @Override
        public void onSurfaceCreated() {
            nFrames=0;
        }

        @Override
        public void onSurfaceChanged(int width, int height){
            nFrames=0;
            rgb[0]=0.0f;
            rgb[1]=0.0f;
            rgb[2]=0.0f;
        }
        @Override
        public void onDrawFrame() {
            nFrames++;
            rgb[1]+=0.02f;
            if(rgb[1]>1){
                rgb[1]=0;
            }
            if(nFrames==2){
                String eglExtensions=EGL14.eglQueryString(EGL14.eglGetCurrentDisplay(),EGL14.EGL_EXTENSIONS);
                System.out.println(eglExtensions);
                isEGL_KHR_reusable_syncAvailable(eglExtensions);
                String gles20Extensions= GLES20.glGetString(GLES20.GL_EXTENSIONS);
                System.out.println(gles20Extensions);
                SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
                SharedPreferences.Editor editor=pref_static.edit();
                editor.putBoolean(getString(R.string.GL_QCOM_tiled_renderingAvailable),isGL_QCOM_tiled_renderingAvailable(gles20Extensions));
                editor.putBoolean(getString(R.string.EGL_KHR_reusable_syncAvailable),isEGL_KHR_reusable_syncAvailable(eglExtensions));
                editor.putBoolean(getString(R.string.EGL_ANDROID_front_buffer_auto_refreshAvailable),isEGL_ANDROID_front_buffer_auto_refreshAvailable(eglExtensions));
                editor.putBoolean(getString(R.string.SingleBufferedSurfaceCreatable),isCurrSurfaceSingleBuffered());
                editor.commit();
            }
            if(nFrames>60){
                SharedPreferences pref_static = getSharedPreferences("pref_static", MODE_PRIVATE);
                SharedPreferences.Editor editor=pref_static.edit();
                editor.putBoolean(getString(R.string.A_FBTester_Stop),true);
                editor.commit();
                finish();
            }
            GLES20.glClearColor(rgb[0],rgb[1],rgb[2],1.0f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT|GLES20.GL_DEPTH_BUFFER_BIT|GLES20.GL_STENCIL_BUFFER_BIT);
            try {Thread.sleep(16);} catch (InterruptedException e) {e.printStackTrace();}
        }

        @Override
        public void onGLSurfaceDestroyed() {

        }

    }

    private boolean isCurrSurfaceSingleBuffered(){
        boolean ret= surfaceHasAttributeWithValue(EGL14.EGL_RENDER_BUFFER, EGL14.EGL_SINGLE_BUFFER);
        if(ret){
            System.out.println("Surface is single buffered");
        }else{
            System.out.println("Surface is not single buffered");
        }
        return ret;
    }
    private boolean surfaceHasAttributeWithValue(int attribute, int value) {
        int[] values = new int[1];
        EGL14.eglQuerySurface(EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY),
                EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW), attribute, values, 0);
        return values[0] == value;
    }
    private boolean isGL_QCOM_tiled_renderingAvailable(String extensions){
        boolean ret=false;
        if(extensions.contains("GL_QCOM_tiled_rendering")){
            ret=true;
            System.out.println("GL_QCOM_tiled_rendering is available");
        }else{
            System.out.println("GL_QCOM_tiled_rendering is not available");
        }
        return ret;
    }
    private boolean isEGL_KHR_reusable_syncAvailable(String extensions){
        boolean ret=false;
        if(extensions.contains("EGL_KHR_reusable_sync")){
            ret=true;
            System.out.println("EGL_KHR_reusable_sync is available");
        }else{
            System.out.println("EGL_KHR_reusable_sync is not available");
        }
        return ret;
    }
    private boolean isEGL_ANDROID_front_buffer_auto_refreshAvailable(String eglExtensions){
        boolean ret=false;
        if(eglExtensions.contains("EGL_ANDROID_front_buffer_auto_refresh")){
            ret=true;
            System.out.println("EGL_ANDROID_front_buffer_auto_refresh is available");
        }else{
            System.out.println("EGL_ANDROID_front_buffer_auto_refresh is not available");
        }
        return ret;
    }

}
