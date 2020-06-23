package constantin.fpv_vr.OSD2;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class CompassLadder extends View {
    private static final String TAG="CompassLadder";
    private static final boolean DEBUG_POSITION=true;
    private final Paint mPaint = new Paint();
    //
    //private final Rect mTextCurrentValue=new Rect();
    private final RectF mTextCurrentValue=new RectF();

    private TextView currentValueView;

    private final int N_LADDER_LINES=8;
    private final int UNITS_BETWEEN_LADDERS=360 / 8; //45

    private float value=0;

    int WIDTH_PX;
    int HEIGHT_PX;

    public CompassLadder(Context context) {
        super(context);
        currentValueView=new TextView(context);
        currentValueView.setText("HI");
        Drawable backgroundColor = new ColorDrawable(Color.argb(0.4f,0.0f,0.0f,0.0f));
        currentValueView.setBackground(backgroundColor);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec,heightMeasureSpec);
        Log.d(TAG,"onMeasure");
        currentValueView.measure(widthMeasureSpec,heightMeasureSpec);
    }


    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        Log.d(TAG,"onLayout "+l+" "+t+" "+r+" "+b+" ");
        currentValueView.layout(l,t,r,b);
    }

    private void drawLadderLine(final Canvas canvas,final float x,final int representedValue){
        float startY=HEIGHT_PX/3.0f;
        float stopY=HEIGHT_PX/3.0f*2;
        canvas.drawLine(x,startY,x,stopY,mPaint);
        //
        final String text=""+representedValue;
        Rect bounds=new Rect();
        mPaint.getTextBounds(text,0,text.length(),bounds);

        canvas.drawText(text,x-bounds.width()/2.0f,HEIGHT_PX,mPaint);
    }

    @SuppressLint("DrawAllocation")
    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        //Log.d(TAG,"onDraw"+canvas.isHardwareAccelerated());
        if(DEBUG_POSITION){
            Helper.drawOutline(canvas,this);
        }

        //currentValueView.draw(canvas);

        value += 0.05f;
        value = value % 360;

        WIDTH_PX = this.getMeasuredWidth();
        HEIGHT_PX = this.getMeasuredHeight();


        final String valueAsString=""+value;
        mPaint.setTextSize(80);
        mPaint.setColor(Color.GREEN);
        //mPaint.getTextBounds(valueAsString,0,valueAsString.length(),mTextCurrentValue);
        mTextCurrentValue.bottom=HEIGHT_PX/3.0f;
        mTextCurrentValue.left=WIDTH_PX/3.0f;
        mTextCurrentValue.top=0;
        mTextCurrentValue.right=WIDTH_PX/3.0f*2;
        Helper.drawOutlineRect(canvas,mTextCurrentValue);
        Helper.drawTextCentered(canvas,mTextCurrentValue,mPaint,Math.round(value));

        mPaint.setColor(Color.YELLOW);
        canvas.drawCircle(WIDTH_PX/2.0f,HEIGHT_PX/2.0f,HEIGHT_PX/50.0f,mPaint);

        //
        mPaint.setStyle(Paint.Style.FILL);
        mPaint.setStrokeWidth(10);
        mPaint.setColor(Color.GREEN);

        final float LADDER_HEIGHT=HEIGHT_PX/2.0f;
        final float DISTANCE_BETWEEEN_LADDERS_X=(float)WIDTH_PX/N_LADDER_LINES;

        mPaint.setTextSize(20);
        final float xPos=(WIDTH_PX/2.0f)+(value % UNITS_BETWEEN_LADDERS) * DISTANCE_BETWEEEN_LADDERS_X /UNITS_BETWEEN_LADDERS ;
        final int representedValue=Helper.roundTo((int)value,UNITS_BETWEEN_LADDERS);

        drawLadderLine(canvas,xPos,representedValue);

        for(int i=0;i<N_LADDER_LINES/2+2;i++){
            float x=xPos - i*DISTANCE_BETWEEEN_LADDERS_X;
            drawLadderLine(canvas,x,representedValue+i*UNITS_BETWEEN_LADDERS);
        }
        for(int i=0;i<N_LADDER_LINES/2+2;i++){
            float x=xPos+ i*DISTANCE_BETWEEEN_LADDERS_X;
            drawLadderLine(canvas,x,representedValue-i*UNITS_BETWEEN_LADDERS);
        }

    }
}
