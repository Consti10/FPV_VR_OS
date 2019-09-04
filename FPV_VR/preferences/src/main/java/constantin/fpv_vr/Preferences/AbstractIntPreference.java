package constantin.fpv_vr.Preferences;

import android.content.Context;
import android.preference.EditTextPreference;
import android.preference.PreferenceManager;
import android.util.AttributeSet;

/**
 * Wrapper for {@link android.preference.EditTextPreference} that exposes additional methods.
 */
public abstract class AbstractIntPreference extends EditTextPreference {
    AbstractIntPreference(Context context) {
        super(context);
    }

    AbstractIntPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    AbstractIntPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onAttachedToHierarchy(PreferenceManager preferenceManager) {
        super.onAttachedToHierarchy(preferenceManager);
    }

    @Override
    protected boolean persistInt(int value) {
        return super.persistInt(value);
    }

    @Override
    protected int getPersistedInt(int defaultReturnValue) {
        return super.getPersistedInt(defaultReturnValue);
    }
}
