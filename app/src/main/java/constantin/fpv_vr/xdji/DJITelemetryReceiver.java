package constantin.fpv_vr.xdji;

import android.app.Activity;
import android.widget.Toast;

import androidx.lifecycle.LifecycleOwner;

import constantin.fpv_vr.Toaster;
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
import dji.sdk.products.Aircraft;

public class DJITelemetryReceiver extends TelemetryReceiver {
    private static final float MPS_TO_KPH=3.6f;

    public <T extends Activity & LifecycleOwner> DJITelemetryReceiver(T parent, long externalGroundRecorder, long externalFileReader) {
        super(parent, externalGroundRecorder,externalFileReader);
        if(DJIApplication.isDJIEnabled(context)){
            setupDJICallbacks();
        }
    }

    private void setupDJICallbacks(){
        final Aircraft aircraft=DJIApplication.getConnectedAircraft();
        if (aircraft==null) {
            Toaster.makeToast(context,"Cannot start telemetry",true);
        } else {
            Toast.makeText(context, "starting dji telemetry", Toast.LENGTH_LONG).show();
            aircraft.getGimbal().setMode(GimbalMode.FPV, new CommonCallbacks.CompletionCallback() {
                @Override
                public void onResult(DJIError djiError) {
                    if (djiError == null) {
                        System.out.println("Set gimbal to X");
                    } else {
                        System.out.println("Cannot set gimbal to X" + djiError.getDescription());
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
            aircraft.getAirLink().setDownlinkSignalQualityCallback(new SignalQualityCallback() {
                @Override
                public void onUpdate(int i) {
                    setDJIDownlinkSignalQuality(nativeInstance, i);
                }
            });
            aircraft.getBattery().setStateCallback(new BatteryState.Callback() {
                @Override
                public void onUpdate(BatteryState state) {
                    setDJIBatteryValues(nativeInstance, state.getChargeRemainingInPercent(), state.getCurrent() * 1000);
                }
            });
            aircraft.getFlightController().setStateCallback(new FlightControllerState.Callback() {
                @Override
                public void onUpdate(FlightControllerState state) {
                    final LocationCoordinate2D home = state.getHomeLocation();
                    final LocationCoordinate3D aircraftLocation = state.getAircraftLocation();
                    final Attitude aircraftAttitude = state.getAttitude();
                    setDJIValues(nativeInstance, aircraftLocation.getLatitude(), aircraftLocation.getLongitude(), aircraftLocation.getAltitude(),
                            -(float) aircraftAttitude.roll, (float) aircraftAttitude.pitch,
                            state.getVelocityX() * MPS_TO_KPH,
                            -state.getVelocityZ() * MPS_TO_KPH,
                            state.getSatelliteCount(), (float) aircraftAttitude.yaw);
                    setHomeLocation(nativeInstance, home.getLatitude(), home.getLongitude(), 0);
                }
            });
                /*aircraft.getAirLink().getWiFiLink().setDataRate(WifiDataRate.RATE_1_MBPS, new CommonCallbacks.CompletionCallback() {
                    @Override
                    public void onResult(DJIError djiError) {
                        debugDJIError("wifi datarate",djiError);
                    }
                });
                aircraft.getCamera().setVideoResolutionAndFrameRate(new ResolutionAndFrameRate(
                        SettingsDefinitions.VideoResolution.RESOLUTION_1280x720, SettingsDefinitions.VideoFrameRate.FRAME_RATE_60_FPS), new CommonCallbacks.CompletionCallback() {
                    @Override
                    public void onResult(DJIError djiError) {
                            debugDJIError("video resolution",djiError);
                    }
                });*/
        }
    }

    private void debugDJIError(final String s,final DJIError djiError){
        if(djiError!=null){
            System.out.println(s+"Error"+djiError.getDescription());
        }else{
            System.out.println(s+"Success");
        }
    }

}
