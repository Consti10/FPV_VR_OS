package constantin.fpv_vr;


public interface VideoPlayerInterface {
    void onVideoRatioChanged(int videoW, int videoH);
    void onVideoFPSChanged(float decFPS);
}
