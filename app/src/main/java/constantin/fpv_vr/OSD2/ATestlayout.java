package constantin.fpv_vr.OSD2;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import java.util.Timer;
import java.util.TimerTask;

// https://developer.android.com/topic/performance/vitals/render


public class ATestlayout extends AppCompatActivity {
    private Timer timer;
    private  MFpvView fpvView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // setContentView(R.layout.activity_testlayout);
        fpvView=new MFpvView(this);
        setContentView(fpvView);

        //layout=findViewById(R.id.layout_test);
    }

    @Override
    protected void onResume(){
        super.onResume();

        timer=new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        final boolean dirty=ATestlayout.this.fpvView.isDirty();
                        //System.out.println("Is dirty:"+dirty);
                        if(!dirty){
                            fpvView.updateUi();
                        }
                    }
                });
            }
        },0,17);
    }

    @Override
    protected void onPause(){
        super.onPause();
        timer.cancel();
        timer.purge();
    }

}