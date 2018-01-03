package constantin.fpv_vr;

import android.opengl.EGL14;

/*******************************************************************************
 * Created by Constantin on 01.01.2017.
 * Chooses a RGBA_8888 (or higher) config with MSAA if mMSAALevel>0.
 * Note: Do not call this config chooser with an unsupported MSAA level. Query valid MSAA levels first, like Activity_TestMSAA
 * Needed by GLSurfaceViewEGL14
 ******************************************************************************/

public class EGL14Config{
    /**
     * Chooses a valid EGL Config for EGL14
     *
     * @param eglDisplay
     *            EGL14 Display
     * @return Resolved config
     */
    public static android.opengl.EGLConfig chooseConfig(final android.opengl.EGLDisplay eglDisplay,int msaaLevel)
    {
        System.out.println("MSAA Level: "+msaaLevel);
        final int[] attribList = {
                EGL14.EGL_RED_SIZE, 8, //
                EGL14.EGL_GREEN_SIZE, 8, //
                EGL14.EGL_BLUE_SIZE, 8, //
                EGL14.EGL_ALPHA_SIZE, 8, //
                EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT, //
                EGL14.EGL_SURFACE_TYPE,EGL14.EGL_WINDOW_BIT,
                EGL14.EGL_SAMPLE_BUFFERS, 1,
                EGL14.EGL_SAMPLES, msaaLevel,
                EGL14.EGL_NONE };
        if(msaaLevel==0){
            attribList[attribList.length-5]=EGL14.EGL_NONE;
        }

        android.opengl.EGLConfig[] configList = new android.opengl.EGLConfig[1];
        final int[] numConfigs = new int[1];

        if (!EGL14.eglChooseConfig(eglDisplay, attribList, 0, configList, 0, configList.length, numConfigs, 0))
        {
            throw new RuntimeException("failed to find valid RGB8888 EGL14 EGLConfig");
        }

        return configList[0];
    }
}