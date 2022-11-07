package com.example.kutallimentapp;

import android.hardware.SensorEvent;

public interface ContractMain {

    interface MainView {
        void onSensorChanged(SensorEvent event);

        void setString(String string);
    }

    interface ArduinoModel {
        interface OnEventListener {
            void onEvent(String string);
        }
        void getArduinoMessage(ContractMain.ArduinoModel.OnEventListener listener);
    }

    interface PresenterMainActivity {
        void onButtonClick();
        void onDestroy();
    }


}