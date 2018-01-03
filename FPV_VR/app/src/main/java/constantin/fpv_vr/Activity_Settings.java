package constantin.fpv_vr;

import android.app.Activity;
import android.content.Context;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import java.util.Set;

public class Activity_Settings extends Activity {
    private Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        // Display the fragment as the main content.
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new MySettingsFragment())
                .commit();
    }

    @Override
    protected void onPause(){
        super.onPause();
        Settings.readFromSharedPreferencesThreaded(mContext);
    }

}
