package constantin.fpv_vr;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;


/***********************************************************
 * Holds static functions that can be called by cpp
 * NOTE: only static functions !!
 ***********************************************************/

public class JNIHelper {

    //decodes a png data byte array into a bitmap, and loads the bitmap into the CURRENTLY BOUND texture unit
    private static void loadPNGIntoTexture(byte[] pngData) {
        Bitmap bmp= BitmapFactory.decodeByteArray(pngData,0,pngData.length);
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0);
        bmp.recycle();
    }
}
