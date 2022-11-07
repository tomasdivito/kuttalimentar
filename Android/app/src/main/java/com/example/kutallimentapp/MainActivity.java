package com.example.kutallimentapp;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements ContractMain.MainView {

    private TextView mensajeArduino;
    private Button botonServirComida;

    ContractMain.PresenterMainActivity presenter;

    SensorManager sensorManager;
    Sensor sensor;
    SensorEventListener sensorEventListener;
    int whip = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mensajeArduino = findViewById(R.id.mensajeArduino);
        botonServirComida = findViewById(R.id.botonServirComida);
        presenter = new PresenterMainActivity(this, new ArduinoModel());

        botonServirComida.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                presenter.onButtonClick();
            }
        });

        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        if (sensor == null) {
            finish();
        }

        sensorEventListener = new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent sensorEvent) {
                float x = sensorEvent.values[0];
                System.out.println("Valor de giro" + x);
                //movemos hacia la derecha
                if (x < -5 && whip == 0) {
                    getWindow().getDecorView().setBackgroundColor(Color.RED);
                    whip++;
                }
                //movemos hacia la izquierda
                else if (x > 5 && whip == 1) {
                    getWindow().getDecorView().setBackgroundColor(Color.BLUE);
                    whip++;
                }

                if (whip == 2) {
                    whip = 0;
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int i) {

            }
        };
        start();
    }

    private void start() {
        sensorManager.registerListener(sensorEventListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void stop() {
        sensorManager.unregisterListener(sensorEventListener);
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        presenter.onDestroy();
    }

    @Override
    public void setString(String string) {
        mensajeArduino.setText(string);
    }

    @Override
    protected void onPause() {
        stop();
        super.onPause();
    }
}