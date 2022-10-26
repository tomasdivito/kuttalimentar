package com.example.kutallimentapp;

public class PresenterMainActivity implements ContractMain.PresenterMainActivity, ContractMain.ArduinoModel.OnEventListener {

    private ContractMain.MainView view;
    private ContractMain.ArduinoModel model;

    public PresenterMainActivity(MainActivity view, ArduinoModel model) {
        this.view = view;
        this.model = model;
    }

    @Override
    public void onButtonClick() {
        model.getArduinoMessage(this);
    }

    @Override
    public void onDestroy() {
        view = null;
    }

    @Override
    public void onEvent(String string) {
        if(view != null){
            view.setString(string);
        }
    }
}
