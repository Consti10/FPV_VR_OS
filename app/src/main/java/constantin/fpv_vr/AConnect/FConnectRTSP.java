package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import constantin.fpv_vr.R;
import constantin.fpv_vr.databinding.ConnectRtspFragmentBinding;

@SuppressWarnings("FieldCanBeLocal")
public class FConnectRTSP extends Fragment implements View.OnClickListener {
    private Context mContext;
    private ConnectRtspFragmentBinding binding;


    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        mContext=getActivity();
        binding=ConnectRtspFragmentBinding.inflate(inflater);
        final SharedPreferences pref_video=mContext.getSharedPreferences("pref_video",Context.MODE_PRIVATE);
        final String url=pref_video.getString(mContext.getString(R.string.VS_FFMPEG_URL),"rtsp://");
        binding.editTextRtspUrl.setText(url);
        binding.editTextRtspUrl.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @SuppressLint("ApplySharedPref")
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                //Toaster.makeToast(mContext,"URL saved",false);
                pref_video.edit().putString(mContext.getString(R.string.VS_FFMPEG_URL),v.getText().toString()).commit();
                return false;
            }
        });
        return binding.getRoot();
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
