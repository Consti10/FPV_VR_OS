package constantin.fpv_vr.XDJI;

import android.app.Activity;
import android.widget.Toast;

import androidx.lifecycle.LifecycleOwner;

import constantin.telemetry.core.TelemetryReceiver;
import constantin.telemetry.core.TelemetrySettings;
import dji.common.airlink.SignalQualityCallback;
import dji.common.battery.BatteryState;
import dji.common.error.DJIError;
import dji.common.flightcontroller.Attitude;
import dji.common.flightcontroller.FlightControllerState;
import dji.common.flightcontroller.LocationCoordinate3D;
import dji.common.gimbal.GimbalMode;
import dji.common.model.LocationCoordinate2D;
import dji.common.util.CommonCallbacks;
import dji.sdk.base.BaseProduct;
import dji.sdk.products.Aircraft;
import dji.sdk.sdkmanager.DJISDKManager;

public class DJITelemetryReceiver extends TelemetryReceiver {

    public <T extends Activity & LifecycleOwner> DJITelemetryReceiver(T parent, long externalGroundRecorder) {
        super(parent, externalGroundRecorder);
        TelemetrySettings.setT_SOURCE(parent,TelemetrySettings.SOURCE_TYPE_EXTERNAL_DJI);
        /////
        if(TelemetrySettings.getT_SOURCE(parent)==TelemetrySettings.SOURCE_TYPE_EXTERNAL_DJI){
            final BaseProduct product = DJISDKManager.getInstance().getProduct();
            final Aircraft aircraft=(Aircraft)product;
            if (product == null || !product.isConnected() || aircraft==null) {
                Toast.makeText(context,"Cannot start dji telemetry",Toast.LENGTH_LONG).show();
            } else {
                Toast.makeText(context,"starting dji telemetry",Toast.LENGTH_LONG).show();
                product.getGimbal().setMode(GimbalMode.FPV, new CommonCallbacks.CompletionCallback() {
                    @Override
                    public void onResult(DJIError djiError) {
                        if(djiError==null){
                            System.out.println("Set gimbal to X");
                        }else{
                            System.out.println("Cannot set gimbal to X"+djiError.getDescription());
                        }
                    }
                });
            /*Rotation.Builder builder = new Rotation.Builder().mode(RotationMode.RELATIVE_ANGLE);
            builder.time(1);
            builder.pitch(0);
            //builder.roll(0); //does not work but should
            //builder.yaw(0); //not supported axis on gimbal (rotate spark instead)
            product.getGimbal().rotate(builder.build(), new CommonCallbacks.CompletionCallback() {
                @Override
                public void onResult(DJIError djiError) {
                    if(djiError==null){
                        System.out.println("Sucesfully rotated gimbal");
                    }else{
                        System.out.println("Cannot rotate gimbal"+djiError.getDescription());
                    }
                }
            });*/
                product.getAirLink().setDownlinkSignalQualityCallback(new SignalQualityCallback() {
                    @Override
                    public void onUpdate(int i) {
                        setDJIDownlinkSignalQuality(nativeInstance,i);
                    }
                });
                product.getBattery().setStateCallback(new BatteryState.Callback() {
                    @Override
                    public void onUpdate(BatteryState state) {
                        setDJIBatteryValues(nativeInstance,state.getChargeRemainingInPercent(),state.getCurrent()*1000);
                    }
                });
                aircraft.getFlightController().setStateCallback(new FlightControllerState.Callback() {
                    @Override
                    public void onUpdate(FlightControllerState state) {
                        final LocationCoordinate2D home=state.getHomeLocation();
                        final LocationCoordinate3D aircraftLocation=state.getAircraftLocation();
                        final Attitude aircraftAttitude=state.getAttitude();
                        setDJIValues(nativeInstance,aircraftLocation.getLatitude(),aircraftLocation.getLongitude(),aircraftLocation.getAltitude(),
                                (float)aircraftAttitude.roll,(float)aircraftAttitude.pitch,state.getVelocityX(),-state.getVelocityZ(),
                                state.getSatelliteCount(),(float)aircraftAttitude.yaw);
                        setHomeLocation(nativeInstance,home.getLatitude(),home.getLongitude(),0);
                    }
                });
            }
        }
    }

}
