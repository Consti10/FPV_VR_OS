package constantin.fpv_vr.XDJI;

import android.app.Application;
import android.content.Context;

import com.secneo.sdk.Helper;

public class DJIApplication extends Application {

    @Override
    protected void attachBaseContext(Context paramContext) {
        super.attachBaseContext(paramContext);
        Helper.install(DJIApplication.this);
    }

}