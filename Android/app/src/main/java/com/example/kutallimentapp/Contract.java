package com.example.kutallimentapp;

public interface Contract {

    interface View {
        void setString(String string);
    }

    interface Model {
        interface OnEventListener {
            void onEvent(String string);
        }
        void getNextName(Contract.Model.OnEventListener listener);
    }

    interface Presenter {
        void onButtonClick();
        void onDestroy();
    }
}
