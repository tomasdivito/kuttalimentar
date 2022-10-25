package com.example.kutallimentapp;

public interface Contract {

    interface View {
        void setString(String string);
    }

    interface ArduinoModel {
        interface OnEventListener {
            void onEvent(String string);
        }
        void getArduinoMessage(Contract.ArduinoModel.OnEventListener listener);
    }

    interface PresenterMainActivity {
        void onButtonClick();
        void onDestroy();
    }

    interface LoginView {
        void setString(String string);
    }

    interface PresenterLogin {
        void onButtonClick();
        void onDestroy();
    }

    interface LoginModel {
        interface onEventListener {
            void onEvent(String string);
        }
        void validatePassword(Contract.LoginModel.OnEventListener listener);
    }
}