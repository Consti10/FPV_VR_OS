package constantin.fpv_vr.OSD2;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.util.Log;
import android.view.View;

// NOTE: DO NOT FORGET TO CALL measure in onMeasure FOR THIS VIEW !
public class DebugOutlineView extends View {
    private static final String TAG="DebugOutlineView";

    public DebugOutlineView(Context context) {
        super(context);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if(true){
            Helper.drawOutline(canvas,this);
        }
        //Log.d(TAG,"onDraw "+getMeasuredWidth()+" "+getMeasuredHeight());
    }

}
