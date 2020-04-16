package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import constantin.fpv_vr.R;

public class FConnectDJI extends Fragment implements View.OnClickListener{
    private Context mContext;

    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.connect_rtsp_fragment, container, false);
        mContext=getActivity();
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
    }


    @SuppressLint("SetTextI18n")
    @Override
    public void onClick(View v) {

    }
}
