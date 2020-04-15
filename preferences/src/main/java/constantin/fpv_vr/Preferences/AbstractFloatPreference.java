package constantin.fpv_vr.Preferences;

import android.content.Context;
import android.preference.EditTextPreference;
import android.preference.PreferenceManager;
import android.util.AttributeSet;

/**
 * Wrapper for {@link android.preference.EditTextPreference} that exposes additional methods.
 */

public abstract class AbstractFloatPreference extends EditTextPreference {
    AbstractFloatPreference(Context context) {
        super(context);
    }

    AbstractFloatPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    AbstractFloatPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onAttachedToHierarchy(PreferenceManager preferenceManager) {
        super.onAttachedToHierarchy(preferenceManager);
    }

    @Override
    protected boolean persistFloat(float value) {
        return super.persistFloat(value);
    }

    @Override
    protected float getPersistedFloat(float defaultReturnValue) {
        return super.getPersistedFloat(defaultReturnValue);
    }
}
