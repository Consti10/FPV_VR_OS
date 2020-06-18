package constantin.fpv_vr;

import androidx.appcompat.app.AppCompatActivity;

import android.content.SharedPreferences;
import android.graphics.Typeface;
import android.os.Bundle;
import android.text.BoringLayout;
import android.text.DynamicLayout;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.StaticLayout;
import android.text.style.ForegroundColorSpan;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

import static android.graphics.Typeface.BOLD_ITALIC;

// https://developer.android.com/topic/performance/vitals/render


public class ATestlayout extends AppCompatActivity {
    private Timer timer;
    private  LinearLayout layout;
    private VerticalLadder heightLadder;
    private VerticalLadder speedLadder;
    private VerticalLadder compassLadder;

    private final ArrayList<TextView> testTextViews=new ArrayList<>();

    private int color1,color2,color3,colorO;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_testlayout);

        final SharedPreferences colorPreferences=getSharedPreferences("pref_osd",MODE_PRIVATE);
        color1=colorPreferences.getInt(getString(R.string.OSD_TEXT_FILL_COLOR1),0);
        color2=colorPreferences.getInt(getString(R.string.OSD_TEXT_FILL_COLOR2),0);
        color3=colorPreferences.getInt(getString(R.string.OSD_TEXT_FILL_COLOR3),0);
        colorO=colorPreferences.getInt(getString(R.string.OSD_TEXT_OUTLINE_COLOR),0);

        layout=findViewById(R.id.layout_test);


        for(int i=0;i<6;i++){
            LinearLayout layout1=new LinearLayout(this);
            layout1.setLayoutParams(new ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT));
            layout1.setOrientation(LinearLayout.HORIZONTAL);
            layout.addView(layout1);
            for(int j=0;j<4;j++){
                TextView textView=new TextView(this);
                textView.setText("Hi "+i);
                textView.setHeight(80);
                textView.setWidth(500);
                //textView.setShadowLayer(1, 0, 0, Color.BLACK);
                textView.setShadowLayer(0.5f, 0, 0, colorO);

                Typeface face = Typeface.create(Typeface.MONOSPACE, BOLD_ITALIC);
                textView.setTypeface(face);
                textView.setGravity(Gravity.CENTER);

                textView.getPaint().setAntiAlias(false);

                //Drawable backgroundColor = new ColorDrawable(Color.argb(0.1f,0.0f,0.0f,0.0f));
               // textView.setBackground(backgroundColor);

                layout1.addView(textView);
                testTextViews.add(textView);

            }
        }
        //
        LinearLayout layout1=new LinearLayout(this);
        layout1.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
        layout1.setOrientation(LinearLayout.HORIZONTAL);
        layout.addView(layout1);

        heightLadder=new VerticalLadder(this);
        heightLadder.setLayoutParams(new LinearLayout.LayoutParams(400,400));
        layout1.addView(heightLadder);

        speedLadder=new VerticalLadder(this);
        speedLadder.setLayoutParams(new LinearLayout.LayoutParams(400,400));
        layout1.addView(speedLadder);

        compassLadder=new VerticalLadder(this);
        compassLadder.setLayoutParams(new LinearLayout.LayoutParams(400,400));
        layout1.addView(compassLadder);


        BoringLayout l;
        StaticLayout l2;
        DynamicLayout l3;
    }

    private void updateUi(){
        System.out.println("Hello update");
        for(int i=0;i<10;i++){
            final int index=new Random().nextInt(testTextViews.size());
            final TextView tv=testTextViews.get(index);

            //SpannableStringBuilder b=new SpannableStringBuilder();
            //b.append("Prefix");
            //b.setSpan()
            //
            Spannable word1 = new SpannableString("Prefix ");
            word1.setSpan(new ForegroundColorSpan(color1), 0, word1.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);


            final int random=new Random().nextInt(1000);
            Spannable word2 = new SpannableString(""+random);
            word2.setSpan(new ForegroundColorSpan(color2), 0, word2.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

            Spannable word3 = new SpannableString(" fps");
            word3.setSpan(new ForegroundColorSpan(color3), 0, word3.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

            tv.setText(word1);
            tv.append(word2);
            tv.append(word3);
        }
        heightLadder.invalidate();
        speedLadder.invalidate();
        compassLadder.invalidate();
        /*for(final TextView tv:testTextViews){
            Spannable word1 = new SpannableString("Prefix ");
            word1.setSpan(new ForegroundColorSpan(color1), 0, word1.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

            final int random=new Random().nextInt(1000);
            Spannable word2 = new SpannableString(""+random);
            word2.setSpan(new ForegroundColorSpan(color2), 0, word2.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

            Spannable word3 = new SpannableString(" fps");
            word3.setSpan(new ForegroundColorSpan(color3), 0, word3.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

            tv.setText(word1);
            tv.append(word2);
            tv.append(word3);

        }*/
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
                        final boolean dirty=ATestlayout.this.layout.isDirty();
                        System.out.println("Is dirty:"+dirty);
                        if(!dirty){
                            updateUi();
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