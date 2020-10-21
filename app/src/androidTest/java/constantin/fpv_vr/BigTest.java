package constantin.fpv_vr;

import androidx.test.espresso.NoMatchingViewException;
import androidx.test.ext.junit.rules.ActivityScenarioRule;
import androidx.test.filters.LargeTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;
import androidx.test.rule.GrantPermissionRule;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import constantin.fpv_vr.main.AMain;

import static androidx.test.espresso.Espresso.onData;
import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.Espresso.pressBack;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static org.hamcrest.CoreMatchers.anything;

@LargeTest
@RunWith(AndroidJUnit4ClassRunner.class)
public class BigTest {

    private static final int WAIT_TIME_LONG=10000;
    private static final int WAIT_TIME_SHORT=1000;
    private static final int SECONDS_10_IN_MS=10*1000;
    private static final int SECONDS_20_IN_MS=10*1000;

    // ActivityScenarioRule is a replacement for ActivityTestRule
    @Rule
    public ActivityScenarioRule<AMain> mActivityTestRule = new ActivityScenarioRule<AMain>(AMain.class);

    @Rule
    public GrantPermissionRule mGrantPermissionRule =
            GrantPermissionRule.grant(
                    //Permissions.BASIC_PERMISSIONS
                    Permissions.ALL_PERMISSIONS
            );

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

    private void clickOkayOnceAvailable(final int maxTimeSeconds){
        final long startTime=System.currentTimeMillis();
        boolean done=false;
        while (!done){
            try{
                onView(withId(android.R.id.button1)).perform(click());
                done=true;
            }catch (NoMatchingViewException ignored){
            }
            long elapsed=System.currentTimeMillis()-startTime;
            if(elapsed>maxTimeSeconds*1000)done=true;
        }
    }

    @Test
    public void basicTest() {
        // The 'start first time' stuff
        try { Thread.sleep(1000); } catch (InterruptedException e) { e.printStackTrace(); }
        try{
            onView(withId(android.R.id.button1)).perform(click());
            // If there is the 'first start or update view' don't forget to clear the second one
            clickOkayOnceAvailable(5);
        }catch (NoMatchingViewException e){
            // Not a first start
            e.printStackTrace();
        }
        try { Thread.sleep(1000); } catch (InterruptedException e) { e.printStackTrace(); }

        // start mono mono & osd, stereo
        openAll3PlayingActivitiesShort();

        //Open AConnect and change receiving mode to manually
        onView(withId(R.id.b_Connect)).perform(click());
        onView(withId(R.id.spinner_connection_type)).perform(click());
        onData(anything()).atPosition(1).perform(click());
        //And check if the test receiver is working properly
        try { Thread.sleep(WAIT_TIME_SHORT); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();
        openAll3PlayingActivitiesShort();

        // Set to DJI and
        // only check if app does not crash with dji enabled
        onView(withId(R.id.b_Connect)).perform(click());
        onView(withId(R.id.spinner_connection_type)).perform(click());
        onData(anything()).atPosition(5).perform(click());
        // Wait for dji registration and so on
        try { Thread.sleep(SECONDS_10_IN_MS); } catch (InterruptedException e) { e.printStackTrace(); }
        pressBack();
    }

}
