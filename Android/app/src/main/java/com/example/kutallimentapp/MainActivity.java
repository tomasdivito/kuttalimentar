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
import android.widget.TextView;
import android.widget.Toast;

import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity implements ContractMain.MainView {
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private TextView mensajeArduino;
    private Button botonServirComida;

    private BluetoothAdapter adapter;
    ContractMain.PresenterMainActivity presenter;

    public static final int MULTIPLE_PERMISSIONS = 10; // code you want.

    //se crea un array de String con los permisos a solicitar en tiempo de ejecucion
    //Esto se debe realizar a partir de Android 6.0, ya que con verdiones anteriores
    //con solo solicitarlos en el Manifest es suficiente
    String[] permissions = new String[]{
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN,
            Manifest.permission.BLUETOOTH_CONNECT,
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.READ_EXTERNAL_STORAGE};

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
        botonServirComida = findViewById(R.id.botonServirComida);

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
        });
        //###########################################################################

        presenter = new PresenterMainActivity(this, new ArduinoModel(), sensorModel);

        botonServirComida.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                presenter.onButtonClick();
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
            return false;
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case MULTIPLE_PERMISSIONS: {
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permissions granted.
                    enableComponent(); // Now you call here what ever you want :)
                } else {
                    String perStr = "";
                    for (String per : permissions) {
                        perStr += "\n" + per;
                    }
                    // permissions list of don't granted permission
                    Toast.makeText(this, "ATENCION: La aplicacion no funcionara " +
                            "correctamente debido a la falta de Permisos", Toast.LENGTH_LONG).show();
                }
                return;
            }
        }
    }

    public void enableComponent() {

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return;
        }
        presenter.connectBluetooth();
        // Move this to the presenter/model
        /*adapter = BluetoothAdapter.getDefaultAdapter();

        String hc05Address = null;
        Log.d("Totii", "Hello");
        Set<BluetoothDevice> pairedDevices = adapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            Log.d("","Found " + pairedDevices.size() + " Devices");
            for (BluetoothDevice device : pairedDevices) {
                if (device.getAddress().equals("00:21:11:01:B9:E9")) {
                    Log.d("", "found");
                    hc05Address = device.getAddress();
                }
                Log.d("",device.getName() + "\n" + device.getAddress() + "\n" + device.getBondState());
            }
        } else {
            Log.d("","No devices found");
        }

        if (hc05Address != null) {
            Log.d("Log", "Alimentador encontrado!");
            try {
                BluetoothDevice hc05 = adapter.getRemoteDevice(hc05Address);
                BluetoothSocket socket = hc05.createRfcommSocketToServiceRecord(BTMODULEUUID);
                socket.connect();
                OutputStream outStream = socket.getOutputStream();
                byte[] buffer = "a".getBytes();
                outStream.write(buffer);
            } catch(Exception e) {
                e.printStackTrace();
            }
        } else {
            Log.d("", "Alimentador no encontrado :(");
        }
        */
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
    public void setString(String string) {
        mensajeArduino.setText(string);
    }

    @Override
    public void onBluetoothStatusChange(String status) {
        mensajeArduino.setText(status);
    }

    @Override
    protected void onPause() {
        presenter.pause();
        super.onPause();
    }
}