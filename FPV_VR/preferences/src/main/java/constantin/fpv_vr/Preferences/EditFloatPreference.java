package constantin.fpv_vr.Preferences;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.util.Log;

import java.util.Objects;

import static android.text.InputType.TYPE_CLASS_NUMBER;
import static android.text.InputType.TYPE_NUMBER_FLAG_DECIMAL;


/**
 * {@link android.preference.DialogPreference} that saves float values to
 * {@link android.content.SharedPreferences}.
 */
public class EditFloatPreference extends AbstractFloatPreference {
    public static final String TAG = EditFloatPreference.class.getSimpleName();

    public EditFloatPreference(Context context) {
        super(context);
        getEditText().setRawInputType(TYPE_CLASS_NUMBER | TYPE_NUMBER_FLAG_DECIMAL);
    }

    public EditFloatPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        getEditText().setRawInputType(TYPE_CLASS_NUMBER | TYPE_NUMBER_FLAG_DECIMAL);
    }

    public EditFloatPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        getEditText().setRawInputType(TYPE_CLASS_NUMBER | TYPE_NUMBER_FLAG_DECIMAL);
    }

    @Override
    protected String getPersistedString(String defaultReturnValue) {
        if(getSharedPreferences().contains(getKey())) {
            float floatValue = getPersistedFloat(0f);
            return String.valueOf(floatValue);
        } else {
            return defaultReturnValue;
        }
    }

    @Override
    protected boolean persistString(String value) {
        //System.out.println("Min:"+MIN_VALUE+" Max:"+MAX_VALUE);
        float floatValue;
        try {
            floatValue = Float.valueOf(value);
        } catch (NumberFormatException e) {
            Log.e(TAG, "Unable to parse preference value: " + value);
            //setSummary("Invalid value");
            return false;
        }

        //setSummary(Float.toString(floatValue));
        return persistFloat(floatValue);
    }

    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        return Float.valueOf(Objects.requireNonNull(a.getString(index))).toString();
    }
}
