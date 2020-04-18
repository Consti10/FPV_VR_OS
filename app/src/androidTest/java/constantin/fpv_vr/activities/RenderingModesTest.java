package constantin.fpv_vr.activities;


import android.content.Intent;

import androidx.test.espresso.NoMatchingViewException;
import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.rule.GrantPermissionRule;
import androidx.test.runner.AndroidJUnit4;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import constantin.fpv_vr.main.AMain;
import constantin.fpv_vr.R;
import constantin.renderingx.core.gles_info.AWriteGLESInfo;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.Espresso.pressBack;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.action.ViewActions.scrollTo;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withText;

@LargeTest
@RunWith(AndroidJUnit4.class)
public class RenderingModesTest {

    private static final int WAIT_TIME_LONG=10000;
    private static final int WAIT_TIME_SHORT=10000;

    @Rule
    public ActivityTestRule<AMain> mActivityTestRule = new ActivityTestRule<>(AMain.class);
    @Rule
    public ActivityTestRule<AWriteGLESInfo> mGLESInfoRule = new ActivityTestRule<>(AWriteGLESInfo.class,false,false);

    @Rule
    public GrantPermissionRule mGrantPermissionRule =
            GrantPermissionRule.grant(
                    "android.permission.ACCESS_FINE_LOCATION",
                    "android.permission.WRITE_EXTERNAL_STORAGE");

    private void startGLESInfo(){
        Intent i = new Intent();
        mGLESInfoRule.launchActivity(i);
        mGLESInfoRule.finishActivity();
    }

    @Test
    public void renderingModesTest() {
        startGLESInfo();
        try {
            onView(withText("Okay")).perform(scrollTo(),click());
            onView(withText("Okay")).perform(scrollTo(),click());
        } catch (NoMatchingViewException ignored) { }


        //start the 3 basic activities, without modifying any settings
        onView(withId(R.id.b_startMonoVideoOnly)).perform(click());
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();
        onView(withId(R.id.b_startMonoVideoOSD)).perform(click());
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();
        onView(withId(R.id.b_startStereo)).perform(click());
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();

        /*
         * Change rendering mode to disable60fpsCap
         */
        /*onView(withId(R.id.b_VRSettings)).perform(click());
        onData(PreferenceMatchers.withTitleText("Stereo/VR rendering 'Hacks'")).perform(scrollTo(),click());
        onData(PreferenceMatchers.withTitleText("Disable 60 OpenGL fps cap")).perform(scrollTo(),click());
        pressBack();
        pressBack();

        onView(withId(R.id.b_startStereo)).perform(click());
        try {
            Thread.sleep(WAIT_TIME_LONG);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        pressBack();*/



        //change rendering mode to 'disable VSYNC"
        /*onView(withId(R.id.b_VRSettings)).perform(click());
        onData(PreferenceMatchers.withTitleText("Stereo/VR rendering 'Hacks'")).perform(scrollTo(),click());
        onData(PreferenceMatchers.withTitleText("Disable VSYNC")).perform(scrollTo(),click());
        pressBack();
        pressBack();

        onView(withId(R.id.b_startStereo)).perform(click());
        try {
            Thread.sleep(WAIT_TIME_LONG);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        pressBack();*/


        //Change rendering mode to 'SuperSync'
        /*onView(withId(R.id.b_VRSettings)).perform(click());
        onData(PreferenceMatchers.withTitleText("Stereo/VR rendering 'Hacks'")).perform(scrollTo(),click());
        onData(PreferenceMatchers.withTitleText("SuperSync")).perform(scrollTo(),click());
        pressBack();
        pressBack();

        onView(withId(R.id.b_startStereo)).perform(click());
        try {
            Thread.sleep(WAIT_TIME_LONG);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        pressBack();*/

    }

}
