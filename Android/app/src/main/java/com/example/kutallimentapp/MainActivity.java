package com.example.kutallimentapp;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

public class MainActivity extends AppCompatActivity implements Contract.View {

    private TextView mensajeArduino;
    private Button botonServirComida;

    Contract.Presenter presenter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mensajeArduino = findViewById(R.id.mensajeArduino);
        botonServirComida = findViewById(R.id.botonServirComida);

        presenter = new Presenter(this, new Model());

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