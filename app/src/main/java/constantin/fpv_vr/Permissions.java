package constantin.fpv_vr;

import android.Manifest;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public class Permissions {
    // These are the permissions for FPV_VR to work
    public static final String[] BASIC_PERMISSIONS=new String[]{
            //Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.ACCESS_FINE_LOCATION
    };

    public static final String[] ALL_PERMISSIONS= BASIC_PERMISSIONS;
}
