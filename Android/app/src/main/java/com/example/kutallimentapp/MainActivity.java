package com.example.kutallimentapp;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Build;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity implements ContractMain.MainView {
    private TextView mensajeArduino;
    private TextView mensajeApp;
    private ImageView dogImage;
    private Button botonServirComida;
    private Button botonConectar;

    ContractMain.PresenterMainActivity presenter;

    public static final int MULTIPLE_PERMISSIONS = 10; // code you want.

    String[] permissions = new String[]{
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN,
            Manifest.permission.BLUETOOTH_CONNECT,
    };

    SensorModel sensorModel;
    SensorManager sensorManager;
    Sensor sensor;

    SensorEventListener sensorEventListener;
    int whip = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mensajeArduino = findViewById(R.id.mensajeArduino);
        mensajeApp = findViewById(R.id.mensajeApp);
        botonServirComida = findViewById(R.id.botonServirComida);
        botonConectar = findViewById(R.id.botonConectar);
        dogImage = findViewById(R.id.dogImage);

        // Instancio el sensor.
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        if (sensor == null) {
            finish();
        }
        sensorModel = new SensorModel(sensorManager,new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent sensorEvent) {
                float x = sensorEvent.values[0];
                float y = sensorEvent.values[1];
                dogImage.setRotationX(x);
                dogImage.setRotationY(y);
                System.out.println("Valor de giro" + x);
                //movemos hacia la derecha
                if (x < -5 && whip == 0) {
                    //getWindow().getDecorView().setBackgroundColor(Color.RED);
                    whip++;
                }
                //movemos hacia la izquierda
                else if (x > 5 && whip == 1) {
                    //getWindow().getDecorView().setBackgroundColor(Color.BLUE);
                    whip++;
                }

                if (whip == 2) {
                    whip = 0;
                }
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int i) {

            }
        });

        presenter = new PresenterMainActivity(this, new ArduinoModel(this), sensorModel);

        botonServirComida.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                presenter.onButtonClick();
            }
        });

        botonConectar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                enableComponent();
            }
        });

        if (checkPermissions()) {
            enableComponent();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (checkPermissions()) {
            enableComponent();
        }
    }

    //Metodo que chequea si estan habilitados los permisos
    private boolean checkPermissions() {
        int result;
        List<String> listPermissionsNeeded = new ArrayList<>();

        //Se chequea si la version de Android es menor a la 6
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return true;
        }


        for (String p : permissions) {
            result = ContextCompat.checkSelfPermission(this, p);
            if (result != PackageManager.PERMISSION_GRANTED) {
                listPermissionsNeeded.add(p);
            }
        }
        if (!listPermissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(this, listPermissionsNeeded.toArray(new String[listPermissionsNeeded.size()]), MULTIPLE_PERMISSIONS);
        }

        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case MULTIPLE_PERMISSIONS: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    //enableComponent(); // Now you call here what ever you want :)
                } else if (grantResults.length > 0 && grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(this, "ATENCION: Tenes que darle permisos de bluetooth!", Toast.LENGTH_LONG).show();
                }
            }
        }
    }

    public void enableComponent() {
        presenter.connectBluetooth();

        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        if (sensor == null) {
            finish();
        }

        sensorEventListener = new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent sensorEvent) {
                float x = sensorEvent.values[0];
                //System.out.println("Valor de giro" + x);
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
    public void updateArduinoState(String string) {
        mensajeArduino.setText(string);
    }

    @Override
    public void updateAppState(String status) {
        mensajeApp.setText(status);
    }

    @Override
    protected void onPause() {
        presenter.pause();
        stop(); // Stop sensor listeners
        super.onPause();
    }
}