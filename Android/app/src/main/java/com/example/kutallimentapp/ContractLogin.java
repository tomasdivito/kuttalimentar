package com.example.kutallimentapp;

public interface ContractLogin {

    interface LoginView {
        void switchActivities();
    }

    interface PresenterLogin {
        void onButtonClick();
        void onDestroy();
    }

    interface LoginModel {
        
    }
}
