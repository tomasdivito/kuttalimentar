package com.example.kutallimentapp;

public interface ContractMain {

    interface MainView {
        void updateArduinoState(String string);
        void updateAppState(String status);
    }

    interface ArduinoModel {
        interface OnEventListener {
            void onEvent(String string);
            void onBluetoothEvent(String string);
        }

        void openFood();
        void connectBluetooth(OnEventListener eventListener);
        void disconnect(OnEventListener eventListener);
    }

    interface PresenterMainActivity {
        void onButtonClick();
        void onDestroy();
        void connectBluetooth();

        void pause();
    }


    public interface SensorModel {
        void pause();
    }
}