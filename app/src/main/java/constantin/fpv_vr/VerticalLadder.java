package constantin.fpv_vr;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.View;

public class VerticalLadder extends View {
    private Paint circlePaint = new Paint();


    public VerticalLadder(Context context) {
        super(context);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        final int WIDTH=this.getMeasuredWidth();
        final int HEIGHT=this.getMeasuredHeight();

        //circlePaint.setStyle(Paint.Style.FILL);
        //circlePaint.setColor(Color.YELLOW);
        //canvas.drawRect(0,this.getMeasuredHeight(),this.getMeasuredWidth(),0,circlePaint);

        //get half of the width and height as we are working with a circle
        int viewWidthHalf = this.getMeasuredWidth()/2;
        int viewHeightHalf = this.getMeasuredHeight()/2;

        circlePaint.setStyle(Paint.Style.FILL);
        circlePaint.setStrokeWidth(10);
        circlePaint.setColor(Color.BLACK);
        for(int i=0;i<10;i++){
            canvas.drawLine(0,HEIGHT/10*i,WIDTH,HEIGHT/10*i,circlePaint);
        }
        circlePaint.setTextSize(80);
        canvas.drawText("N km",WIDTH/2,HEIGHT/2,circlePaint);

        //subtract ten so that it has some space around it
        /*int radius = 0;
        if(viewWidthHalf>viewHeightHalf)
            radius=viewHeightHalf-10;
        else
            radius=viewWidthHalf-10;

        circlePaint.setStyle(Paint.Style.FILL);
        circlePaint.setAntiAlias(true);

        circlePaint.setColor(Color.GREEN);

        canvas.drawCircle(viewWidthHalf, viewHeightHalf, radius, circlePaint);*/

    }
}
