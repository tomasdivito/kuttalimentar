package com.example.kutallimentapp;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements ContractMain.MainView {

    private TextView mensajeArduino;
    private Button botonServirComida;

    ContractMain.PresenterMainActivity presenter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mensajeArduino = findViewById(R.id.mensajeArduino);
        botonServirComida = findViewById(R.id.botonServirComida);
        presenter = new PresenterMainActivity(this, new ArduinoModel());

        botonServirComida.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                presenter.onButtonClick();
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        presenter.onDestroy();
    }

    @Override
    public void setString(String string){
        mensajeArduino.setText(string);
    }
}