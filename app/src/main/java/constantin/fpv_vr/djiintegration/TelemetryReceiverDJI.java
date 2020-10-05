package constantin.fpv_vr.djiintegration;

import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

import constantin.fpv_vr.Toaster;
import constantin.telemetry.core.TelemetryReceiver;
import constantin.telemetry.core.TelemetrySettings;
import dji.common.airlink.SignalQualityCallback;
import dji.common.airlink.WiFiFrequencyBand;
import dji.common.battery.BatteryState;
import dji.common.error.DJIError;
import dji.common.flightcontroller.Attitude;
import dji.common.flightcontroller.FlightControllerState;
import dji.common.flightcontroller.LocationCoordinate3D;
import dji.common.gimbal.GimbalMode;
import dji.common.model.LocationCoordinate2D;
import dji.common.remotecontroller.HardwareState;
import dji.common.util.CommonCallbacks;
import dji.sdk.airlink.WiFiLink;
import dji.sdk.products.Aircraft;
import dji.sdk.remotecontroller.RemoteController;

public class TelemetryReceiverDJI extends TelemetryReceiver {
    private static final float MPS_TO_KPH=3.6f;
    private int qualityUpPercentage;
    private int qualityDownPercentage;

    public TelemetryReceiverDJI(AppCompatActivity parent, long externalGroundRecorder, long externalFileReader) {
        super(parent, externalGroundRecorder,externalFileReader);
        if(DJIApplication.isDJIEnabled(context)){
            TelemetrySettings.setT_SOURCE(parent,TelemetrySettings.SOURCE_TYPE_EXTERNAL_DJI);
            setupDJICallbacks();
        }
    }

    private void setupDJICallbacks(){
        final Aircraft aircraft=DJIApplication.getConnectedAircraft();
        if (aircraft==null) {
            Toaster.makeToast(context,"Cannot start telemetry",true);
            return;
        }
        final RemoteController remoteController=aircraft.getRemoteController();
        //Toaster.makeToast(context, "starting dji telemetry", true);
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
                qualityDownPercentage=i;
                setDJISignalQuality(nativeInstance,qualityUpPercentage,qualityDownPercentage);
            }
        });
        aircraft.getAirLink().setUplinkSignalQualityCallback(new SignalQualityCallback() {
            @Override
            public void onUpdate(int i) {
                qualityUpPercentage=i;
                setDJISignalQuality(nativeInstance,qualityUpPercentage,qualityDownPercentage);
            }
        });
        aircraft.getBattery().setStateCallback(new BatteryState.Callback() {
            @Override
            public void onUpdate(BatteryState state) {
                setDJIBatteryValues(nativeInstance, state.getChargeRemainingInPercent(), state.getCurrent() * 1000,state.getVoltage());
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
        aircraft.getFlightController().setMaxFlightHeight(1000, new CommonCallbacks.CompletionCallback() {
            @Override
            public void onResult(DJIError djiError) {
                debugDJIError("Set max flight height",djiError);
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
        final WiFiLink wiFiLink=aircraft.getAirLink().getWiFiLink();
        if(wiFiLink!=null){
            wiFiLink.getFrequencyBand(new CommonCallbacks.CompletionCallbackWith<WiFiFrequencyBand>() {
                @Override
                public void onSuccess(WiFiFrequencyBand wiFiFrequencyBand) {
                    Toaster.makeToast(context,"Frequency is "+DJIHelper.frequencyBandToString(wiFiFrequencyBand));
                }
                @Override
                public void onFailure(DJIError djiError) {
                    Toaster.makeToast(context,"cannot get frequency");
                }
            });
        }
        if(remoteController!=null){
            remoteController.setHardwareStateCallback(new HardwareState.HardwareStateCallback() {
                @Override
                public void onUpdate(HardwareState hardwareState) {
                    final HardwareState.Button functionButton=hardwareState.getFunctionButton();
                    if(functionButton!=null && functionButton.isClicked()){
                       setDJIFunctionButtonClicked(nativeInstance);
                    }
                }
            });
        }
    }


    private static void debugDJIError(final String s,final DJIError djiError){
        if(djiError!=null){
            System.out.println(s+"Error"+djiError.getDescription());
        }else{
            System.out.println(s+"Success");
        }
    }


}
