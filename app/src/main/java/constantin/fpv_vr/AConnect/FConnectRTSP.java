package constantin.fpv_vr.AConnect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import constantin.fpv_vr.R;
import constantin.fpv_vr.Settings.SJ;
import constantin.fpv_vr.Toaster;

@SuppressWarnings("FieldCanBeLocal")
public class FConnectRTSP extends Fragment implements View.OnClickListener {
    private Context mContext;

    private EditText mRtspUrlEditText;
    private Button mTestRTSP1Button;
    private Button mTestRTSP2Button;
    private TextView mTestReceiverTestView;
    //
    private Button mRTSP_SETUP1_Button;
    private Button mRTSP_SETUP2_Button;
    private TextView mTestReceiverTextView;


    @Override
    @SuppressLint("SetTextI18n")
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.connect_rtsp_fragment, container, false);
        mContext=getActivity();

        mTestRTSP1Button=rootView.findViewById(R.id.testRTSP1Button);
        mTestRTSP1Button.setOnClickListener(this);
        mTestRTSP2Button=rootView.findViewById(R.id.testRTSP2Button);
        mTestRTSP2Button.setOnClickListener(this);
        mTestReceiverTestView =rootView.findViewById(R.id.testRTSPStatusTextView);
        //
        mRTSP_SETUP1_Button =rootView.findViewById(R.id.rtspSETUP1_Button);
        mRTSP_SETUP1_Button.setOnClickListener(this);
        mRTSP_SETUP2_Button =rootView.findViewById(R.id.rtspSETUP2_Button);
        mRTSP_SETUP2_Button.setOnClickListener(this);
        //
        mTestReceiverTextView=rootView.findViewById(R.id.testReceiverTextView);
        //
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
