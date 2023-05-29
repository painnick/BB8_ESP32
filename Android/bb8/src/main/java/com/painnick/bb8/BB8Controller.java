package com.painnick.bb8;

import android.os.Process;

import org.json.JSONException;

public class BB8Controller {

    private static final String TAG = "BB8Controller";

    private final String BB8Host = "http://192.168.5.18";

    private final SingleThreadHandlerExecutor executor;

    private final BB8Api bb8Api;

    private Boolean started = false;

    private Boolean stopping = true;

    private boolean isLedOn = false;

    private int lastAngle = 90;

    public int getLastAngle() {
        return lastAngle;
    }

    public BB8Controller(WebImageConsumer webImageListener, BB8ApiConsumer apiListener) {
        executor = new SingleThreadHandlerExecutor("BB8Controller", Process.THREAD_PRIORITY_DEFAULT);
        bb8Api = new BB8Api(BB8Host);
        bb8Api.setNewBitmapListener(bitmap -> {
            webImageListener.onNewBitmap(bitmap);
            executor.execute(this::run);
        });
        bb8Api.setApiListener(response -> {
            if (response.has("angle")) {
                try {
                    lastAngle = response.getInt("angle");
                } catch (JSONException e) {
                    // Something
                }
            }
            apiListener.onResponse(response);
        });
    }

    public void start() {
        stopping = false;
        started = true;
        bb8Api.start();
    }

    public void run() {
        if (started && !stopping) {
            bb8Api.reloadImage();
        }
    }

    public void pause() {
        stopping = true;
    }

    public void resume() {
        if (started) {
            stopping = false;
            executor.execute(this::run);
        }
    }

    public void moveLeft(int degree, boolean found) {
        bb8Api.moveLeft(degree, found);
    }

    public void moveRight(int degree, boolean found) {
        bb8Api.moveRight(degree, found);
    }

    public void ledOn(boolean forced) {
        if (forced || !isLedOn) {
            bb8Api.led(10);
        }
        isLedOn = true;
    }

    public void ledOff(boolean forced) {
        if (forced || isLedOn) {
            bb8Api.led(0);
        }
        isLedOn = false;
    }


}
