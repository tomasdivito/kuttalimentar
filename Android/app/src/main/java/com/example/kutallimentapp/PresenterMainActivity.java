package com.example.kutallimentapp;

public class PresenterMainActivity implements Contract.PresenterMainActivity, Contract.Model.OnEventListener {

    private Contract.View mainView;
    private Contract.Model model;

    public PresenterMainActivity(Contract.View mainView, Contract.Model model) {
        this.mainView = mainView;
        this.model = model;
    }

    @Override
    public void onButtonClick() {
        model.getArduinoMessage(this);
    }

    @Override
    public void onDestroy() {
        mainView = null;
    }

    @Override
    public void onEvent(String string) {
        if(mainView != null){
            mainView.setString(string);
        }
    }
}
