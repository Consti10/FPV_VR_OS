package constantin.fpv_vr.OSD2;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Canvas;
import android.graphics.Typeface;
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

import androidx.viewpager.widget.ViewPager;

import java.util.ArrayList;
import java.util.Random;

import constantin.fpv_vr.R;

import static android.graphics.Typeface.BOLD;

public class MFpvView extends LinearLayout {

    private VerticalLadder heightLadder;
    private VerticalLadder speedLadder;
    private CompassLadder compassLadder;

    private final ArrayList<TextView> testTextViews=new ArrayList<>();

    private int color1,color2,color3,colorO;

    private void addTextViews(){
        final Context context=getContext();
        for(int i=0;i<2;i++){
            LinearLayout layout1=new LinearLayout(context);
            layout1.setLayoutParams(new ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT));
            layout1.setOrientation(LinearLayout.HORIZONTAL);
            addView(layout1);
            for(int j=0;j<4;j++){
                MTextView textView=new MTextView(context);
                textView.setText("Hi "+i);
                textView.setHeight(80);
                textView.setWidth(500);
                //textView.setLayoutParams(new TableLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT, 1f));
                //textView.setShadowLayer(1, 0, 0, Color.BLACK);
                //textView.setShadowLayer(1f, 0, 0, Color.BLACK);

                Typeface face = Typeface.create(Typeface.MONOSPACE, BOLD);
                textView.setTypeface(face);
                textView.setGravity(Gravity.CENTER);

                textView.getPaint().setAntiAlias(false);

                //Drawable backgroundColor = new ColorDrawable(Color.argb(0.4f,0.0f,0.0f,0.0f));
                //textView.setBackground(backgroundColor);
                ViewPager.DecorView v;

                layout1.addView(textView);
                testTextViews.add(textView);
            }
        }
    }

    public MFpvView(Context context) {
        super(context);
        setOrientation(VERTICAL);
        setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        //setBackgroundResource(R.drawable.screenshot_x);

        //
        final SharedPreferences colorPreferences=context.getSharedPreferences("pref_osd", Context.MODE_PRIVATE);
        color1=colorPreferences.getInt(context.getString(R.string.OSD_TEXT_FILL_COLOR1),0);
        color2=colorPreferences.getInt(context.getString(R.string.OSD_TEXT_FILL_COLOR2),0);
        color3=colorPreferences.getInt(context.getString(R.string.OSD_TEXT_FILL_COLOR3),0);
        colorO=colorPreferences.getInt(context.getString(R.string.OSD_TEXT_OUTLINE_COLOR),0);
        //
        addTextViews();
        //
        LinearLayout layout1=new LinearLayout(context);
        layout1.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
        layout1.setOrientation(LinearLayout.HORIZONTAL);

        addView(layout1);

        heightLadder=new VerticalLadder(context);
        heightLadder.setLayoutParams(new LinearLayout.LayoutParams(300,600));
        layout1.addView(heightLadder);

        compassLadder=new CompassLadder(context);
        compassLadder.setLayoutParams(new LinearLayout.LayoutParams(600,300));
        layout1.addView(compassLadder);

        speedLadder=new VerticalLadder(context);
        speedLadder.setLayoutParams(new LinearLayout.LayoutParams(300,600));
        layout1.addView(speedLadder);

        addTextViews();

        BoringLayout l;
        StaticLayout l2;
        DynamicLayout l3;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        //
    }

    public void updateUi(){
        System.out.println("Hello update");
        for(int i=0;i<4;i++){
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
            //OutlineSpan outlineSpan=new OutlineSpan(Color.BLACK,1);
            //word3.setSpan(outlineSpan,0,word3.length(),Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

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


}
