package com.example.kutallimentapp;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import androidx.appcompat.app.AppCompatActivity;

public class LoginActivity extends AppCompatActivity implements ContractLogin.LoginView {

    Button loginButton;
    EditText passwordText;
    ContractLogin.PresenterLogin presenter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        loginButton = findViewById(R.id.login_button);
        passwordText = findViewById(R.id.textPassword);

        presenter = new PresenterLogin(this, new LoginModel(getPreferences(Context.MODE_PRIVATE)));

        presenter.checkLoginStatus();

        loginButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                presenter.onButtonClick(passwordText.getText().toString());
            }
        });
    }

    @Override
    public void onLoginErrored() {

    }

    @Override
    public void onLoginSuccessful() {
        Intent loginIntent = new Intent(this, MainActivity.class);
        startActivity(loginIntent);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        presenter.onDestroy();
    }
}
