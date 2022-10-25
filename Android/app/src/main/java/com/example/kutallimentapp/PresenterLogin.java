package com.example.kutallimentapp;

import android.content.Intent;

public class PresenterLogin implements ContractLogin.PresenterLogin {

    private ContractLogin.LoginView view;
    private ContractLogin.LoginModel model;

    public PresenterLogin(LoginActivity view, LoginModel model) {
        this.view = view;
        this.model = model;
    }

    @Override
    public void onButtonClick() {
        view.switchActivities();
    }

    @Override
    public void onDestroy() {
        view = null;
    }



}
