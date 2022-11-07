package com.example.kutallimentapp;

public interface ContractLogin {

    interface LoginView {
        void onLoginErrored();
        void onLoginSuccessful();
    }

    interface PresenterLogin {
        void checkLoginStatus();
        void onButtonClick(String password);
        void onDestroy();
    }

    interface LoginModel {
        interface EventListener {
            void onCredentialsChecked(Boolean authenticated);
            void onLoginDetected();
        }

        void validatePassword(ContractLogin.LoginModel.EventListener presenter, String password);
        void checkLoginStatus(ContractLogin.LoginModel.EventListener presenter);
    }

}
