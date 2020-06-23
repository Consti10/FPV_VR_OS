package constantin.fpv_vr.OSD2;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;
import android.view.View;

public class Helper {
    public static final Paint mDebugRectPaint=setupDebugPaint();
    private static final Rect tmp=new Rect();
    private static RectF tmp2=new RectF();

    public static Paint setupDebugPaint(){
        final Paint paint=new Paint();
        paint.setStyle(Paint.Style.STROKE);
        paint.setColor(Color.YELLOW);
        paint.setStrokeWidth(2);
        return paint;
    }

    public static void drawOutline(Canvas canvas, View v){
        final int WIDTH_PX=v.getMeasuredWidth();
        final int HEIGHT_PX=v.getMeasuredHeight();
        canvas.drawRect(0,HEIGHT_PX,WIDTH_PX,0,mDebugRectPaint);
    }

    public static void drawOutlineRect(Canvas canvas, RectF rectF){
        canvas.drawRect(rectF,mDebugRectPaint);
    }
    // For the compass the displayed value is always in the range 0...360
    public static void drawTextCentered(final Canvas canvas,final RectF rectF,final Paint paint,final int value){
        final String s=""+value+"°";
        paint.getTextBounds(s,0,s.length(),tmp);
        //final float paddingLeftTopBottom=rectF.height()-tmp.height();
        //if(paddingLeftTopBottom<=0){
        //    Log.d("TextHelper","no padding");
        //}

        final float textX=rectF.left + (rectF.width()-tmp.width())/2.0f;
        final float textY=rectF.bottom - (rectF.height()-tmp.height())/2.0f;

        canvas.drawText(s,textX,textY,paint);

        //canvas.drawRect(tmp,mDebugRectPaint);
    }

    // round to the next smaller multiple of n
    // Example: roundTo(9,10) = 0
    static int roundTo(final int value,final int n){
        return (value / n) * n;
    }
}
