package constantin.fpv_vr.play_stereo;
/* ************************************************************************
 * Renders Video & OSD Side by Side.
 * Pipeline h.264-->image on screen:
 * h.264 nalus->VideoDecoder->SurfaceTexture-(updateTexImage)->Texture->Rendering with OpenGL
 ***************************************************************************/

import android.os.Bundle;

import constantin.fpv_vr.AirHeadTrackingSender;
import constantin.fpv_vr.VideoTelemetryComponent;
import constantin.renderingx.core.views.VrActivity;
import constantin.renderingx.core.views.VrView;
import constantin.renderingx.core.vrsettings.ASettingsVR;

public class AStereoVR extends VrActivity {
    //Components use the android LifecycleObserver. Since they don't need forwarding of
    //onPause / onResume it looks so empty here

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        VrView vrView=new VrView(this);
        if(ASettingsVR.getVR_RENDERING_MODE(this)>=2){
            vrView.enableSuperSync();
        }
        final VideoTelemetryComponent videoTelemetryComponent=new VideoTelemetryComponent(this);
        final GLRStereoVR mGLRStereoVR = new GLRStereoVR(this,videoTelemetryComponent.getTelemetryReceiver(),vrView.getGvrApi().getNativeGvrContext());
        videoTelemetryComponent.setIVideoParamsChanged(mGLRStereoVR);

        vrView.getPresentationView().setRenderer(mGLRStereoVR,videoTelemetryComponent.configure2());
        vrView.getPresentationView().setISecondaryContext(mGLRStereoVR);

        vrView.setIOnEmulateTrigger(new VrView.IEmulateTrigger(){
            @Override
            public void onEmulateTrigger() {
                videoTelemetryComponent.getTelemetryReceiver().incrementOsdViewMode();
            }
        });


        setContentView(vrView);
        AirHeadTrackingSender airHeadTrackingSender = AirHeadTrackingSender.createIfEnabled(this,vrView.getGvrApi());
    }

    @Override
    protected void onResume(){
        super.onResume();
        //Debug.startMethodTracing();
    }

    @Override
    protected void onPause(){
        super.onPause();
        //Debug.stopMethodTracing();
    }

}