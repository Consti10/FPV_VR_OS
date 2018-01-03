package constantin.fpv_vr;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import java.io.IOException;
import java.io.InputStream;


public class MyTestFileFragment extends Fragment {

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.my_testfile_fragment, container, false);
        final ImageView iv = (ImageView) rootView.findViewById(R.id.imageView);
        final String imageFileName = "testvideoscreenshot.jpg";
        iv.setImageBitmap(getBitmapFromAssets(imageFileName));
        return rootView;
    }

    @Override
    public void onPause() {
        super.onPause();
    }
    @Override
    public void onResume() {
        super.onResume();
    }

    // Custom method to get assets folder image as bitmap
    private Bitmap getBitmapFromAssets(String fileName){
        AssetManager am = getActivity().getAssets();
        InputStream is = null;
        try{
            is = am.open(fileName);
        }catch(IOException e) {
            e.printStackTrace();
        }
        return BitmapFactory.decodeStream(is);
    }
}
