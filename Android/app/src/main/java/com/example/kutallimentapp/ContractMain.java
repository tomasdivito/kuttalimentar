package com.example.kutallimentapp;

public interface ContractMain {

    interface MainView {
        void updateArduinoState(String string);
        void updateAppState(String status);
        void updateWeightState(String weight);
    }

    interface ArduinoModel {
        interface OnEventListener {
            void onEvent(String string);
            void onBluetoothEvent(String string);
            void onWeightEvent(String weight);
        }

        void openFood();
        void connectBluetooth(OnEventListener eventListener);
        void disconnect(OnEventListener eventListener);
    }

    interface PresenterMainActivity {
        void serveFood();
        void onDestroy();
        void connectBluetooth();

        void pause();
    }


    public interface SensorModel {
        void pause();
    }
}