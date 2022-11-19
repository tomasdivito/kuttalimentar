package com.example.kutallimentapp;

import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

public class SensorModel implements ContractMain.SensorModel{

    public SensorManager sensorManager;
    public Sensor sensor;
    public SensorEventListener sensorEventListener;


    public SensorModel (SensorManager sensorManager, SensorEventListener sensorEventListener){
        this.sensorManager = sensorManager;
        this.sensor = this.sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        this.sensorEventListener = sensorEventListener;
        start();
    }

    private void start() {
        sensorManager.registerListener(sensorEventListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void stop() {
        sensorManager.unregisterListener(sensorEventListener);
    }

    @Override
    public void pause(){
        stop();
    }

}
