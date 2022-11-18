package com.example.kutallimentapp;

import android.os.Handler;
import android.os.Message;

public class PresenterMainActivity implements ContractMain.PresenterMainActivity, ContractMain.ArduinoModel.OnEventListener {

    private ContractMain.MainView view;
    private ContractMain.ArduinoModel model;

    public PresenterMainActivity(MainActivity view, ArduinoModel model) {
        this.view = view;
        this.model = model;
    }

    @Override
    public void onButtonClick() {
        model.openFood();
    }

    @Override
    public void onDestroy() {
        view = null;
    }

    @Override
    public void connectBluetooth() {
        model.connectBluetooth(this);
    }

    @Override
    public void onEvent(String string) {
        if(view != null){
            view.setString(string);
        }
    }
}
