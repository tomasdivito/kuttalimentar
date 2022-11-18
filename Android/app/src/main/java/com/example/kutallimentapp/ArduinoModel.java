package com.example.kutallimentapp;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import androidx.core.app.ActivityCompat;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

public class ArduinoModel implements ContractMain.ArduinoModel {
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private Handler handler;
    BluetoothThread btThread;
    OnEventListener eventListener;

    public ArduinoModel() {
        handler = Handler_Msg_Principal();
    }

    private Handler Handler_Msg_Principal() {
        return new Handler() {
            public void handleMessage(Message msg) {
                //si se recibio un msj del hilo secundario
                if (msg.what == 4) {
                    //voy concatenando el msj
                    String readMessage = (String) msg.obj;
                    eventListener.onEvent(readMessage);
                }
            }
        };
    }

    @Override
    public void openFood() {
        if (btThread != null) {
            try {
                btThread.write("a");
            } catch (IOException exception) {
                Log.e("bt", "error al mandar mensaje");
            }
        }
    }

    @Override
    public void connectBluetooth(OnEventListener listener) {
        eventListener = listener;
        BluetoothDevice hc05 = null;
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        Set<BluetoothDevice> pairedDevices = adapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            Log.d("","Found " + pairedDevices.size() + " Devices");
            for (BluetoothDevice device : pairedDevices) {
                if (device.getAddress().equals("00:21:11:01:B9:E9")) {
                    hc05 = device;
                    Log.d("", "found");
                }
                Log.d("",device.getName() + "\n" + device.getAddress() + "\n" + device.getBondState());
            }
        } else {
            eventListener.onEvent("No devices found");
            Log.d("","No devices found");
        }

        if (hc05 != null) {
            eventListener.onEvent("Alimentador encontrado");
            Log.d("Log", "Alimentador encontrado!");
            try {
                BluetoothSocket socket = hc05.createInsecureRfcommSocketToServiceRecord(BTMODULEUUID);

                try {
                    socket.connect();
                } catch (IOException e) {
                    e.printStackTrace();
                    eventListener.onEvent("Error al conectarse con el alimentador");
                    return;
                }

                // bluetooth encontrado creo socket
                btThread = new BluetoothThread(socket);

                btThread.start();


            } catch(Exception e) {
                e.printStackTrace();
            }
        } else {
            Log.d("", "Alimentador no encontrado :(");
        }
    }

    @Override
    public void getArduinoMessage(final OnEventListener listener) {
        listener.onEvent(getMessage());
    }

    private String getMessage() {
        return "Comida servida!";
    }

    public class BluetoothThread extends Thread {
        private InputStream inputStream;
        private OutputStream outputStream;

        public BluetoothThread(BluetoothSocket socket) throws IOException {
            this.inputStream = socket.getInputStream();
            this.outputStream = socket.getOutputStream();
        }

        @Override
        public void run() {
            byte[] buffer = new byte[256];
            int bytes;

            while (true) {
                try {
                    bytes = inputStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);

                    Message msg = Message.obtain();
                    msg.what = 4;
                    msg.obj = readMessage;
                    handler.sendMessage(msg);
                } catch (IOException e) {
                    break;
                }
            }
        }

        public void write(String content) throws IOException {
            byte[] buffer = content.getBytes();
            outputStream.write(buffer);
        }

    }
}