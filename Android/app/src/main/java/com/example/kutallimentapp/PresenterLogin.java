package com.example.kutallimentapp;

public class PresenterLogin implements ContractLogin.PresenterLogin, ContractLogin.LoginModel.EventListener {

    private ContractLogin.LoginView view;
    private ContractLogin.LoginModel model;

    public PresenterLogin(LoginActivity view, LoginModel model) {
        this.view = view;
        this.model = model;
    }

    @Override
    public void checkLoginStatus() {
        model.checkLoginStatus(this);
    }

    @Override
    public void onButtonClick(String password) {
        model.validatePassword(this, password);
    }

    @Override
    public void onCredentialsChecked(Boolean authenticated) {
        if (authenticated) {
            view.onLoginSuccessful();
        } else {
            view.onLoginErrored();
        }
    }

    @Override
    public void onLoginDetected() {
        view.onLoginSuccessful();
    }

    @Override
    public void onDestroy() {
        view = null;
    }
}
