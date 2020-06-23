package constantin.fpv_vr.OSD2;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.util.Log;
import android.widget.TextView;


@SuppressLint("AppCompatCustomView")
public class MTextView extends TextView {
    private static final String TAG="MTextView";
    public MTextView(Context context) {
        super(context);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec,heightMeasureSpec);
        //Log.d(TAG,"onMeasure");
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        //Log.d(TAG,"onLayout");
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        // text outline https://stackoverflow.com/questions/3182393/android-textview-outline-text
        //for (int i = 0; i < 5; i++) {
        //    super.onDraw(canvas);
        //}
        //Log.d(TAG,"onDraw"+canvas.isHardwareAccelerated());
    }

}
