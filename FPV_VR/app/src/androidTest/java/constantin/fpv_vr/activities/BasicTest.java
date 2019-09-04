package constantin.fpv_vr.activities;

import androidx.test.espresso.NoMatchingViewException;
import androidx.test.espresso.matcher.PreferenceMatchers;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.rule.GrantPermissionRule;
import androidx.test.runner.AndroidJUnit4;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import constantin.fpv_vr.AMain.AMain;
import constantin.fpv_vr.R;

import static androidx.test.espresso.Espresso.onData;
import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.Espresso.pressBack;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.action.ViewActions.scrollTo;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withText;
import static org.hamcrest.CoreMatchers.anything;

@LargeTest
@RunWith(AndroidJUnit4.class)
public class BasicTest {

    private static final int WAIT_TIME_LONG=10000;
    private static final int WAIT_TIME_SHORT=1000;

    @Rule
    public ActivityTestRule<AMain> mActivityTestRule = new ActivityTestRule<>(AMain.class);

    @Rule
    public GrantPermissionRule mGrantPermissionRule =
            GrantPermissionRule.grant(
                    "android.permission.ACCESS_FINE_LOCATION",
                    "android.permission.WRITE_EXTERNAL_STORAGE");

    //start the 3 basic activities, without modifying any settings
    private void openAll3PlayingActivitiesShort(){
        onView(withId(R.id.b_startMonoVideoOnly)).perform(click());
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();
        onView(withId(R.id.b_startMonoVideoOSD)).perform(click());
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();
        onView(withId(R.id.b_startStereo)).perform(click());
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();
    }

    @Test
    public void basicTest() {

        try {
            onView(withText("Okay")).perform(scrollTo(),click());
            onView(withText("Okay")).perform(scrollTo(),click());
        } catch (NoMatchingViewException ignored) { }


        //Change rendering mode to disable60fpsCap and start stereo activity
        onView(withId(R.id.b_VRSettings)).perform(click());
        onData(PreferenceMatchers.withTitleText("Stereo/VR rendering 'Hacks'")).perform(scrollTo(),click());
        onData(PreferenceMatchers.withTitleText("Disable 60 OpenGL fps cap")).perform(scrollTo(),click());
        pressBack();
        pressBack();
        onView(withId(R.id.b_startStereo)).perform(click());
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();

        //Open AConnect and change receiving mode to manually
        onView(withId(R.id.b_Connect)).perform(click());
        onView(withId(R.id.spinner_connection_type)).perform(click());
        onData(anything()).atPosition(1).perform(click());
        //And check if the test receiver is working properly
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();

        //Then,open the mono/stereo activities for the last time
        openAll3PlayingActivitiesShort();
    }

}
