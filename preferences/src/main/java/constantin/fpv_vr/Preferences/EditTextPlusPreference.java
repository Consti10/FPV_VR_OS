package constantin.fpv_vr.Preferences;

import android.content.Context;
import android.preference.EditTextPreference;
import android.util.AttributeSet;

/**
 * {@link android.preference.EditTextPreference} that displays its current value as the summary.
 */
public class EditTextPlusPreference extends EditTextPreference {
    public EditTextPlusPreference(Context context) {
        super(context);
    }

    public EditTextPlusPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public EditTextPlusPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected boolean persistString(String value) {
        //setSummary(value);
        return super.persistString(value);
    }
}
