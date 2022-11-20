package com.example.kutallimentapp;

public class PresenterMainActivity implements ContractMain.PresenterMainActivity, ContractMain.ArduinoModel.OnEventListener {

    private ContractMain.MainView view;
    private ContractMain.ArduinoModel model;
    private ContractMain.SensorModel sensor;

    public PresenterMainActivity(MainActivity view, ArduinoModel model, ContractMain.SensorModel sensor) {
        this.view = view;
        this.model = model;
        this.sensor = sensor;
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
            view.updateArduinoState(string);
        }
    }

    @Override
    public void pause(){
        this.model.disconnect(this);
        sensor.pause();
    }
}
