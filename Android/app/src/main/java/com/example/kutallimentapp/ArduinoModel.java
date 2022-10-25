package com.example.kutallimentapp;

public class ArduinoModel implements Contract.ArduinoModel {

    @Override
    public void getArduinoMessage(final OnEventListener listener) {
        listener.onEvent(getMessage());
    }

    private String getMessage() {
        return "Comida servida!";
    }
}