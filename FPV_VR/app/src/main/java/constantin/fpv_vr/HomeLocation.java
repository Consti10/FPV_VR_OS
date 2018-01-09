package constantin.fpv_vr;


import android.annotation.SuppressLint;
import android.content.Context;
import android.location.Location;
import android.os.Bundle;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.location.LocationListener;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;

/**
 * Created by Constantin on 05.01.2018.
 */

public class HomeLocation implements GoogleApiClient.ConnectionCallbacks,GoogleApiClient.OnConnectionFailedListener,LocationListener {
    private Context mContext;
    private GoogleApiClient mGoogleApiClient;
    private Location mCurrentHomeLocation;
    private HomeLocationChanged mHomeLocationChangedI;

    public HomeLocation(Context context,HomeLocationChanged homeLocationChanged){
        mContext=context;
        mGoogleApiClient=new GoogleApiClient.Builder(mContext)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                .addApi(LocationServices.API)
                .build();
        mHomeLocationChangedI=homeLocationChanged;
    }


    public void resume(){
        mGoogleApiClient.connect();
    }

    public void pause(){
        //LocationServices.FusedLocationApi.removeLocationUpdates(mGoogleApiClient, (LocationListener) this);
        if(mGoogleApiClient.isConnected()){
            mGoogleApiClient.disconnect();
        }
    }


    private void printCurrentLoc(){
        System.out.println("Lat:"+mCurrentHomeLocation.getLatitude()+" Lon:"+mCurrentHomeLocation.getLongitude()+" Alt:"
                +mCurrentHomeLocation.getAltitude()+" Accuracy:"+mCurrentHomeLocation.getAccuracy()+" Provider:"
                +mCurrentHomeLocation.getProvider());
    }

    @SuppressLint("MissingPermission")
    @Override
    public void onConnected(Bundle bundle) {
        mCurrentHomeLocation=LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);
        try{
            LocationRequest locReq=LocationRequest.create();
            locReq.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY);
            locReq.setInterval(1000);
            //locReq.setFastestInterval(1000);
            LocationServices.FusedLocationApi.requestLocationUpdates(mGoogleApiClient,locReq,this);
        }catch (Exception e){e.printStackTrace();}
        if(mHomeLocationChangedI!=null && mCurrentHomeLocation!=null){
            mHomeLocationChangedI.onHomeLocationChanged(mCurrentHomeLocation);
        }
        printCurrentLoc();
    }

    @Override
    public void onLocationChanged(Location location){
        mCurrentHomeLocation=location;
        if(mCurrentHomeLocation.getAccuracy()<10){
            //this is accurate enough
            mGoogleApiClient.disconnect();
            System.out.println("Accurate home position received");
        }
        if(mHomeLocationChangedI!=null && mCurrentHomeLocation!=null){
            mHomeLocationChangedI.onHomeLocationChanged(mCurrentHomeLocation);
        }
        printCurrentLoc();
    }

    @Override
    public void onConnectionSuspended(int i) {

    }

    @Override
    public void onConnectionFailed(ConnectionResult connectionResult) {
        Toaster.makeToast(mContext,"Cannot get home location. Specific OSD features might not work",true);
    }

    interface HomeLocationChanged{
        void onHomeLocationChanged(Location location);
    }
}

