package constantin.fpv_vr.Settings;

import android.content.Context;

import com.google.vr.sdk.base.Distortion;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

public class VRSettingsHelper {

    //This one has to run on the UI thread, and is therefore called in onCreate() of
    //AStereoNormal & AStereoSuperSYNC. This means when changing the headset, the distortion params only change when
    //the activity is restarted
    public static float[] getUndistortionCoeficients(Context c){
        GvrView view=new GvrView(c);
        final GvrViewerParams params=view.getGvrViewerParams();
        float[] ret=getUndistortionCoeficients(params);
        System.out.println(coeficientsToString(ret));
        view.shutdown();
        return ret;
        //System.out.println("Model: "+params.getModel());
    }


    private static float[] getUndistortionCoeficients(final GvrViewerParams params){
        //Because of vertex displacement distortion correction
        //float mFOV=45.0f;
        float fovY_full=params.getLeftEyeMaxFov().getBottom()+params.getLeftEyeMaxFov().getTop();
        float fovX_full=params.getLeftEyeMaxFov().getLeft()+params.getLeftEyeMaxFov().getRight();
        System.out.println(params.getModel()+"  "+"FOV Y"+fovY_full+"FOV X"+fovX_full+params.getDistortion().toString());
        System.out.println("FOV:"+params.getLeftEyeMaxFov().toString());
        Distortion distortion=params.getDistortion();

        return calculateUndistortionCoeficients(distortion, fovY_full/2.0f );
    }


    private static float[] calculateUndistortionCoeficients(Distortion distortion,final float FOV_HALF) {
       // distortion=Distortion.parseFromProtobuf(new float[]{0.5f,0});

        float[] results = new float[6+1];
        float maxFovHalfAngle = (float) (FOV_HALF * Math.PI / 180.0f);
        float maxRadiusLens = distortion.distortInverse(maxFovHalfAngle);
        //maxRadiusLens=1;
        if(FOV_HALF<60.0f){
            maxRadiusLens = maxRadiusLens*1.5f;
        }
        Distortion inverse = distortion.getApproximateInverseDistortion(maxRadiusLens,6);
        if(FOV_HALF<60.0f){
            results[0] = maxRadiusLens*1.2f;
        }else{
            results[0] = maxRadiusLens;
        }
        results[1] = inverse.getCoefficients()[0];
        results[2] = inverse.getCoefficients()[1];
        results[3] = inverse.getCoefficients()[2];
        results[4] = inverse.getCoefficients()[3];
        results[5] = inverse.getCoefficients()[4];
        results[6] = inverse.getCoefficients()[5];

        System.out.println("Distort "+inverse.distort(1));
        System.out.println("As in shader:"+ distort(results[0],results[1],results[2],results[3],results[4],results[5],results[6]));

        /*float max= distort(maxRadiusLens,results[1],results[2],results[3],results[4],results[5],results[6]);
        for(float i=0;i<1;i+=0.01){
            float val=distort(maxRadiusLens+i,results[1],results[2],results[3],results[4],results[5],results[6]);
            if(val>max){
                results[0]=maxRadiusLens+i;
                System.out.println("Add"+i);
            }else{
                System.out.println("was"+val);
            }
        }*/
        //Distort 0.61365926
        //As in shader:-0.21330465


        //System.out.println("Distortion factor at 1 is "+distortion.distortionFactor(1));
        //System.out.println("Distortion factor at 1 is "+distortion.distortionFactor(1));

        return results;
    }

    private static String coeficientsToString(float[] coeficients){
        StringBuilder s=new StringBuilder();
        for(int i=0;i<7;i++){
            s.append(coeficients[i]);
            s.append(", ");
        }
        return s.toString();
    }

    /*public void GetLeftEyeVisibleTanAngles(float[] result) {
        // Tan-angles from the max FOV.
        float fovLeft = Mathf.Tan(-device.maxFOV.outer * Mathf.Deg2Rad);
        float fovTop = Mathf.Tan(device.maxFOV.upper * Mathf.Deg2Rad);
        float fovRight = Mathf.Tan(device.maxFOV.inner * Mathf.Deg2Rad);
        float fovBottom = Mathf.Tan(-device.maxFOV.lower * Mathf.Deg2Rad);
        // Viewport size.
        float halfWidth = screen.width / 4;
        float halfHeight = screen.height / 2;
        // Viewport center, measured from left lens position.
        float centerX = device.lenses.separation / 2 - halfWidth;
        float centerY = -VerticalLensOffset;
        float centerZ = device.lenses.screenDistance;
        // Tan-angles of the viewport edges, as seen through the lens.
        float screenLeft = device.distortion.distort((centerX - halfWidth) / centerZ);
        float screenTop = device.distortion.distort((centerY + halfHeight) / centerZ);
        float screenRight = device.distortion.distort((centerX + halfWidth) / centerZ);
        float screenBottom = device.distortion.distort((centerY - halfHeight) / centerZ);
        // Compare the two sets of tan-angles and take the value closer to zero on each side.
        result[0] = Math.Max(fovLeft, screenLeft);
        result[1] = Math.Min(fovTop, screenTop);
        result[2] = Math.Min(fovRight, screenRight);
        result[3] = Math.Max(fovBottom, screenBottom);
    }*/


    public static float distort(float r2, float k0, float k1, float k2, float k3, float k4, float k5){
        float ret=0;
        ret = r2 * (ret + k5);
        ret = r2 * (ret + k4);
        ret = r2 * (ret + k3);
        ret = r2 * (ret + k2);
        ret = r2 * (ret + k1);
        ret = r2 * (ret + k0);
        return ret;
    }
}
