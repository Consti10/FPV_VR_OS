package constantin.fpv_vr.play_stereo;

import android.util.Log;

public class AvgCalculator {
    private long sumUs=0;
    private long sumCount=0;

    public void add(long durationNS){
        final long durationUS=durationNS/1000;
        if(durationUS>=0){
            sumUs+=durationUS;
            sumCount++;
        }else{
            Log.d("AvgCalculator","Duration is negative "+durationUS);
        }
    }
    public long getAvg_us(){
        if(sumCount==0)return 0;
        return sumUs / sumCount;
    }

    // milliseconds & float is the most readable format
    public float getAvg_ms(){
        return (float)getAvg_us()/1000.0f;
    }

    public void reset(){
        sumUs=0;
        sumCount=0;
    }

}
