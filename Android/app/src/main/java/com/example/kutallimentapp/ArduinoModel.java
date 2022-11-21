package com.example.kutallimentapp;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;

public class ArduinoModel implements ContractMain.ArduinoModel {
    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private static final int MSG_BT = 4;
    private static final int MSG_CLOSE = 5;

    private Handler handler;
    private Context context;
    private BluetoothThread btThread;
    private ContractMain.ArduinoModel.OnEventListener eventListener;
    private BluetoothSocket arduinoSocket;

    public ArduinoModel(AppCompatActivity activity) {
        this.context = activity;
        handler = Handler_Msg_Principal();
    }

    private Handler Handler_Msg_Principal() {
        return new Handler() {
            public void handleMessage(Message msg) {
                //si se recibio un msj del hilo secundario
                if (msg.what == MSG_BT) {
                    //voy concatenando el msj
                    String readMessage = (String) msg.obj;
                    String bluetoothEvent = "";
                    switch (readMessage) {
                        case "A":
                            bluetoothEvent = "Sirviendo comida...";
                            break;
                        case "I":
                            bluetoothEvent = "Cargando porcion...";
                            break;
                        case "S":
                            bluetoothEvent = "Porción servida";
                            break;
                        case "C":
                            bluetoothEvent = "Porción lista para servir";
                            break;
                        case "F":
                            bluetoothEvent = "Falta comida en el alimentador!";
                            break;
                        default:
                            bluetoothEvent = "";
                            break;
                    }
                    if (bluetoothEvent.equals("")) {
                        eventListener.onWeightEvent(readMessage);
                    } else {
                        eventListener.onBluetoothEvent(bluetoothEvent);
                    }
                }

                if (msg.what == MSG_CLOSE) {
                    //eventListener.onEvent("DESCONECTADO");
                }
            }
        };
    }

    @Override
    public void openFood() {
        if (btThread != null) {
            btThread.write("a");
        }
    }

    @Override
    public void connectBluetooth(ContractMain.ArduinoModel.OnEventListener listener) {
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        // Presenter as the event listener
        eventListener = listener;

        if (arduinoSocket != null) {
            try {
                arduinoSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        BluetoothDevice hc05 = null;
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        Set<BluetoothDevice> pairedDevices = adapter.getBondedDevices();

        if (pairedDevices.size() > 0) {
            for (BluetoothDevice device : pairedDevices) {
                if (device.getAddress().equals("00:21:11:01:B9:E9")) {
                    hc05 = device;
                }
            }
        } else {
            eventListener.onEvent("CONECTATE AL BLUETOOTH Y AL HC05");
            return;
        }

        if (hc05 != null) {
            eventListener.onEvent("Conectado al alimentador");
            try {
                arduinoSocket = hc05.createInsecureRfcommSocketToServiceRecord(BTMODULEUUID);

                try {
                    arduinoSocket.connect();
                } catch (IOException e) {
                    e.printStackTrace();
                    eventListener.onEvent("Error al conectarse con el alimentador");
                    return;
                }

                // bluetooth encontrado creo socket
                btThread = new BluetoothThread(arduinoSocket);

                btThread.start();


            } catch(Exception e) {
                e.printStackTrace();
            }
        } else {
            eventListener.onEvent("CONECTATE AL HC05");
            Log.d("", "Alimentador no encontrado :(");
        }
    }

    @Override
    public void disconnect(final ContractMain.ArduinoModel.OnEventListener eventListener) {
        if (arduinoSocket == null) {
            return;
        }
        try {
            arduinoSocket.close();
            btThread.stopThread();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public class BluetoothThread extends Thread {
        private InputStream inputStream;
        private OutputStream outputStream;
        private boolean running;

        public BluetoothThread(BluetoothSocket socket) throws IOException {
            this.inputStream = socket.getInputStream();
            this.outputStream = socket.getOutputStream();
        }

        @Override
        public void run() {
            byte[] buffer = new byte[256];
            int bytes;
            running = true;

            while (running) {
                try {
                    bytes = inputStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);

                    Message msg = Message.obtain();
                    msg.what = 4;
                    msg.obj = readMessage;
                    handler.sendMessage(msg);
                } catch (IOException e) {

                    // Send close message to the handler
                    Message msg = Message.obtain();
                    msg.what = MSG_CLOSE;
                    handler.sendMessage(msg);

                    e.printStackTrace();
                    break;
                }
            }
        }

        public void write(String content) {
            byte[] buffer = content.getBytes();
            try {
                outputStream.write(buffer);
            } catch (IOException e) {
                running = false;

                // Send close message to the handler
                Message msg = Message.obtain();
                msg.what = MSG_CLOSE;
                handler.sendMessage(msg);

                e.printStackTrace();
            }
        }

        synchronized public void stopThread() {
            running = false;
        }
    }
}