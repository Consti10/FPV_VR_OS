package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.widget.Toast;

/**
 * Created by Constantin on 04.11.2017.
 * Toast a Toast ;)
 */

public class Toaster {
    public static void makeToast(final Context context,final String message, final boolean longMessage) {
        ((Activity)context).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, message, longMessage ? Toast.LENGTH_LONG : Toast.LENGTH_SHORT).show();
            }
        });
    }
}
