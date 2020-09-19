package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;

import constantin.fpv_vr.connect.AConnect;
import constantin.fpv_vr.main.AMain;

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
    public static void makeToast(final Context context,final String message) {
        makeToast(context,message,false);
    }

    public static void makeHelperAlertDialog(final Context c,final String message){
        new AlertDialog.Builder(c).setMessage(message).
                setPositiveButton("Okay", null).show();
                //.setNegativeButton("No",null).show();
    }
}
