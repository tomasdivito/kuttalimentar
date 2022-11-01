package com.example.kutallimentapp;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.util.Set;

public class MainActivity extends AppCompatActivity implements ContractMain.MainView {

    private TextView mensajeArduino;
    private Button botonServirComida;

    private BluetoothAdapter adapter;

    ContractMain.PresenterMainActivity presenter;

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

        // Move this to the presenter/model
        adapter = BluetoothAdapter.getDefaultAdapter();
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
        Set<BluetoothDevice> pairedDevices = adapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            Log.d("","Found " + pairedDevices.size() + " Devices");
            for (BluetoothDevice device : pairedDevices) {
                Log.d("",device.getName() + "\n" + device.getAddress());
            }
        } else {
            Log.d("","No devices found");
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        presenter.onDestroy();
    }

    @Override
    public void setString(String string){
        mensajeArduino.setText(string);
    }
}