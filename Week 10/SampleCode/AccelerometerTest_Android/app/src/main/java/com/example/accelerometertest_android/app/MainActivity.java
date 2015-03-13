package com.example.accelerometertest_android.app;

import android.os.Handler;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.hardware.*;
import android.widget.*;


public class MainActivity extends ActionBarActivity implements SensorEventListener {
    // Create our sensor objects
    private SensorManager mSensorManager;
    private Sensor mAccelerometer;

    // This used for creating our fake data creation thread
    private Handler mHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Create the sensorManager
        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        // Create the accelerometer
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        // Tell the sensor manager to have the accelerometer talk to us on a certain delay
        mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_UI);

        // Get the button object from the layout, and set it's onclick behavior
        Button button1 = (Button)findViewById(R.id.button);
        button1.setOnClickListener(new OnClickListener(){
            public void onClick(View v) {
                TextView tb = (TextView)findViewById(R.id.outputText);
                tb.setText("Hello, EE 590!");
            }
        });

        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // We don't do anything when the accelerometer accuracy is changed
    }

    public void onSensorChanged(SensorEvent event) {
        // Get the output text view that we talk to
        TextView outputText = (TextView)findViewById(R.id.outputText);
        //outputText.setText("Sensor: " + event.values[0]);

        // Get the progress bar that we talk to
        ProgressBar pb = (ProgressBar)findViewById(R.id.progressBar);
        pb.setProgress((int)(event.values[0]*10) + 50);
    }

}
