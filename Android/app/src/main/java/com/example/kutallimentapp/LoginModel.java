package com.example.kutallimentapp;

import android.content.SharedPreferences;

public class LoginModel implements ContractLogin.LoginModel {
    private SharedPreferences preferences;

    public LoginModel(SharedPreferences preferences) {
        this.preferences = preferences;
    }

    @Override
    public void validatePassword(EventListener presenter, String password) {
        String hardcoded_password = "root";

        Boolean authenticated = hardcoded_password.equals(password);
        if (authenticated) {
            SharedPreferences.Editor editor = preferences.edit();
            editor.putInt("logged_in", 1);
            editor.apply();
        }

        presenter.onCredentialsChecked(authenticated);
    }

    @Override
    public void checkLoginStatus(EventListener presenter) {
        int logged_in = preferences.getInt("logged_in", 0);

        if (logged_in == 1) {
            presenter.onLoginDetected();
        }
    }
}
