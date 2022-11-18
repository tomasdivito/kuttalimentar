package com.example.kutallimentapp;

import android.os.Handler;

public interface ContractMain {

    interface MainView {
        void setString(String string);
        void onBluetoothStatusChange(String status);
    }

    interface ArduinoModel {
        interface OnEventListener {
            void onEvent(String string);
        }
        void openFood();
        void connectBluetooth(OnEventListener eventListener);
        void getArduinoMessage(ContractMain.ArduinoModel.OnEventListener listener);
    }

    interface PresenterMainActivity {
        void onButtonClick();
        void onDestroy();
        void connectBluetooth();
    }


}