package com.example.kutallimentapp;

public interface ContractMain {

    interface MainView {
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