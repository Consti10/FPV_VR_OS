package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.support.design.widget.Snackbar;
import android.view.View;
import android.widget.Toast;



public class Snacker {

    public static void makeSnackBar1(final Context activityContext,final String message){
        ((Activity)activityContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final Snackbar snackbar;
                //final View v=((Activity) activityContext).findViewById(R.id.spinner_nav);
                final View v=((Activity) activityContext).getWindow().getDecorView();
                //final View v = ((Activity) activityContext).getWindow().findViewById(R.id.content);
                if(v!=null){
                    snackbar=Snackbar.make(v, message, Snackbar.LENGTH_INDEFINITE);
                    snackbar.setAction("OK", new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            snackbar.dismiss();
                        }
                    });
                    snackbar.show();
                }
            }
        });
    }

    public static void makeSnackBarForSpinner(final Context activityContext, final String message){
        ((Activity)activityContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final Snackbar snackbar;
                final View v=((Activity) activityContext).findViewById(R.id.spinner_nav);
                //final View v=((Activity) activityContext).getWindow().getDecorView();
                if(v!=null){
                    snackbar=Snackbar.make(v, message, Snackbar.LENGTH_INDEFINITE);
                    snackbar.setAction("OK", new View.OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            snackbar.dismiss();
                        }
                    });
                    snackbar.show();
                }
            }
        });
    }


}
