package constantin.fpv_vr.Preferences;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.util.Log;

import java.util.Objects;

import static android.text.InputType.TYPE_CLASS_NUMBER;

/**
 * {@link android.preference.DialogPreference} that saves integer values to
 * {@link android.content.SharedPreferences}.
 */
public class EditIntPreference extends AbstractIntPreference {
    public static final String TAG = EditIntPreference.class.getSimpleName();

    public EditIntPreference(Context context) {
        super(context);
        getEditText().setRawInputType(TYPE_CLASS_NUMBER);
    }

    public EditIntPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        getEditText().setRawInputType(TYPE_CLASS_NUMBER);
    }

    public EditIntPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        getEditText().setRawInputType(TYPE_CLASS_NUMBER);
    }

    @Override
    protected String getPersistedString(String defaultReturnValue) {
        if(getSharedPreferences().contains(getKey())) {
            int intValue = getPersistedInt(0);
            return String.valueOf(intValue);
        } else {
            return defaultReturnValue;
        }
    }

    @Override
    protected boolean persistString(String value) {
        int intValue;
        try {
            intValue = Integer.valueOf(value);
        } catch (NumberFormatException e) {
            Log.e(TAG, "Unable to parse preference value: " + value);
            //setSummary("Invalid value");
            return false;
        }

        //setSummary(value);
        return persistInt(intValue);
    }

    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        return Integer.decode(Objects.requireNonNull(a.getString(index))).toString();
    }
}
