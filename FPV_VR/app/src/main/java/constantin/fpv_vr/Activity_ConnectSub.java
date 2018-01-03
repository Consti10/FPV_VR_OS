package constantin.fpv_vr;

import android.app.Activity;
import android.content.SharedPreferences;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;

public class Activity_ConnectSub extends Activity{
    //SharedPreferences pref_connect;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //pref_connect=getSharedPreferences("pref_connect", MODE_PRIVATE);
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new NetworkSettingsFrag())
                .commit();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
       //pref_connect.registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    protected void onPause(){
        super.onPause();
        //pref_connect.unregisterOnSharedPreferenceChangeListener(this);
        Settings.readFromSharedPreferencesThreaded(this);
    }

    public static class NetworkSettingsFrag extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            //addPreferencesFromResource(R.xml.pref_connect);
            PreferenceManager prefMgr = getPreferenceManager();
            //System.out.println("ZZZ"+prefMgr.getSharedPreferencesName());
            prefMgr.setSharedPreferencesName("pref_connect");
            prefMgr.setSharedPreferencesMode(MODE_PRIVATE);
            //System.out.println("ZZZ"+prefMgr.getSharedPreferencesName());

            addPreferencesFromResource(R.xml.pref_connect);
            getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onActivityCreated(Bundle savedInstanceState) {
            super.onActivityCreated(savedInstanceState);
            /*View v=getView();
            if(v!=null){
                v.setBackgroundColor(Color.WHITE);
            }*/
        }

        @Override
        public void onResume() {
            super.onResume();
            getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onPause() {
            super.onPause();
            getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onDetach() {
            super.onDetach();
            //when we are done with the network settings, we do not return back to the main screen, but to the Activity_connect screen
            /*Intent mConnectI=new Intent();
            mConnectI.setClass(getActivity(),Activity_Connect.class);
            startActivity(mConnectI);*/
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
           // sharedPreferences.edit().commit();
            //System.out.println("CHANGED:"+sharedPreferences.getString(key,""));
            //sharedPreferences.edit().putString(key,sharedPreferences.getString(key,"0")).commit();
        }
    }
}
