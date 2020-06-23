package constantin.fpv_vr.OSD2;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.Log;
import android.view.View;
// Canvas coordinate system:
// https://medium.com/over-engineering/getting-started-with-drawing-on-the-android-canvas-621cf512f4c7

public class VerticalLadder extends View {
    private static final String TAG="VerticalLadder";
    private static final boolean DEBUG_POSITION=true;
    private final Paint mDebugRectPaint=new Paint();
    private final Paint mPaint = new Paint();
    private final Rect mTextCurrentValue=new Rect();
    private RectF mLOL;

    private final int N_LADDER_LINES=10;
    private final int UNITS_BETWEEN_LADDERS=10;
    private final int RANGE=N_LADDER_LINES*UNITS_BETWEEN_LADDERS;

    private float value=0;


    public VerticalLadder(Context context) {
        super(context);
        mDebugRectPaint.setStyle(Paint.Style.STROKE);
        mDebugRectPaint.setColor(Color.YELLOW);
        mDebugRectPaint.setStrokeWidth(2);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec,heightMeasureSpec);
        Log.d(TAG,"onMeasure");
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        Log.d(TAG,"onLayout");
    }

    private void drawLadderLine(final Canvas canvas,final float y,final float width,final int representedValue){
        if(representedValue<0){
            mPaint.setColor(Color.RED);
        }else{
            mPaint.setColor(Color.YELLOW);
        }
        canvas.drawLine(0,y,width,y,mPaint);
        final String text=""+representedValue;
        Rect bounds=new Rect();
        mPaint.getTextBounds(text,0,text.length(),bounds);
        RectF bounds2=new RectF(bounds);
        bounds2.offsetTo(width,y-bounds.height()/2.0f);
        //
        if(!RectF.intersects(mLOL,bounds2)){
            canvas.drawText(""+representedValue,width,y+bounds.height()/2.0f ,mPaint);
            //canvas.drawRect(bounds2,mDebugRectPaint);
        }
    }

    @SuppressLint("DrawAllocation")
    @Override
    protected void onDraw(Canvas canvas) {
        value+=0.05f;

        final int WIDTH_PX=this.getMeasuredWidth();
        final int HEIGHT_PX=this.getMeasuredHeight();
        if(DEBUG_POSITION){
           Helper.drawOutline(canvas,this);
        }
        // should be multiple
        final float LADDER_LINES_WIDTH=WIDTH_PX/4.0f;

        final float DISTANCE_BETWEEEN_LADDERS_PX=(float)HEIGHT_PX/N_LADDER_LINES;

        //circlePaint.setStyle(Paint.Style.FILL);
        //circlePaint.setColor(Color.YELLOW);
        //canvas.drawRect(0,this.getMeasuredHeight(),this.getMeasuredWidth(),0,circlePaint);

        mPaint.setStyle(Paint.Style.FILL);
        mPaint.setStrokeWidth(10);
        mPaint.setColor(Color.GREEN);

        //canvas.save();
        //canvas.restore();

        //final float y=UNITS_BETWEEN_LADDERS*value;
        //canvas.drawLine(0,y,WIDTH,y,mPaint);

        //for(int i=0;i<10;i++){
        //    canvas.drawLine(0,HEIGHT/10*i,WIDTH,HEIGHT/10*i, mPaint);
        //}
        final String valueAsString=""+value;
        mPaint.setTextSize(80);
        mPaint.getTextBounds(valueAsString,0,valueAsString.length(),mTextCurrentValue);

        canvas.drawText(valueAsString,LADDER_LINES_WIDTH,HEIGHT_PX/2.0f + mTextCurrentValue.height()/2.0f , mPaint);
        mLOL=new RectF(mTextCurrentValue);
        mLOL.offsetTo(LADDER_LINES_WIDTH,HEIGHT_PX/2.0f-mTextCurrentValue.height()/2.0f);
        canvas.drawRect(mLOL,mDebugRectPaint);

        mPaint.setColor(Color.YELLOW);
        canvas.drawCircle(LADDER_LINES_WIDTH,HEIGHT_PX/2,WIDTH_PX/50,mPaint);


        // calculate the position and the represented value
        // (e.g. the value displayed right next to the ladder line)
        //
        final float yPos=(HEIGHT_PX/2.0f)+(value % UNITS_BETWEEN_LADDERS) * DISTANCE_BETWEEEN_LADDERS_PX /UNITS_BETWEEN_LADDERS ;
        final int representedValue=Helper.roundTo((int)value,UNITS_BETWEEN_LADDERS);
        mPaint.setTextSize(DISTANCE_BETWEEEN_LADDERS_PX);

        drawLadderLine(canvas,yPos,LADDER_LINES_WIDTH,representedValue);

        for(int i=0;i<N_LADDER_LINES/2+2;i++){
            float y=yPos - i*DISTANCE_BETWEEEN_LADDERS_PX;
            drawLadderLine(canvas,y,LADDER_LINES_WIDTH,representedValue+i*UNITS_BETWEEN_LADDERS);
        }
        for(int i=0;i<N_LADDER_LINES/2+2;i++){
            float y=yPos+ i*DISTANCE_BETWEEEN_LADDERS_PX;
            drawLadderLine(canvas,y,LADDER_LINES_WIDTH,representedValue-i*UNITS_BETWEEN_LADDERS);
        }

        /*final Paint paint=new Paint();
        paint.setColor(Color.TRANSPARENT);
        paint.setStyle(Paint.Style.FILL);
        paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
        canvas.drawRect(mLOL,paint);*/

        //subtract ten so that it has some space around it
        /*
        //get half of the width and height as we are working with a circle
        int viewWidthHalf = this.getMeasuredWidth()/2;
        int viewHeightHalf = this.getMeasuredHeight()/2;
        int radius = 0;
        if(viewWidthHalf>viewHeightHalf)
            radius=viewHeightHalf-10;
        else
            radius=viewWidthHalf-10;

        circlePaint.setStyle(Paint.Style.FILL);
        circlePaint.setAntiAlias(true);

        circlePaint.setColor(Color.GREEN);

        canvas.drawCircle(viewWidthHalf, viewHeightHalf, radius, circlePaint);*/
        //invalidate();
    }
}
