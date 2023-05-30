package com.painnick.bb8;

import android.os.Process;

import java.util.Date;

enum MOTOR_DIRECTION {
    NONE,
    LEFT,
    RIGHT
}

public class BB8Controller {

    private static final String TAG = "BB8Controller";

    private final String BB8Host = "http://192.168.5.18";

    private final SingleThreadHandlerExecutor executor;

    private final BB8Api bb8Api;

    private Boolean started = false;

    private Boolean stopping = true;

    private MOTOR_DIRECTION direction = MOTOR_DIRECTION.NONE;
    private Date lastReqTime;

    public BB8Controller(WebImageConsumer webImageListener, BB8ApiConsumer apiListener) {
        executor = new SingleThreadHandlerExecutor("BB8Controller", Process.THREAD_PRIORITY_DEFAULT);
        bb8Api = new BB8Api(BB8Host);
        bb8Api.setNewBitmapListener(bitmap -> {
            webImageListener.onNewBitmap(bitmap);
            executor.execute(this::run);
        });
        bb8Api.setApiListener(apiListener);

        lastReqTime = new Date();
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

    public void moveLeft(boolean found, boolean forced) {
        if (direction != MOTOR_DIRECTION.LEFT) {
            lastReqTime = new Date();
            bb8Api.moveLeft(found);
        } else {
            Date now = new Date();
            if (forced) {
                lastReqTime = now;
                bb8Api.moveLeft(found);
            } else {
                long lostMs = (now.getTime() - lastReqTime.getTime());
                if (lostMs > 1000) {
                    lastReqTime = now;
                    bb8Api.moveLeft(found);
                }
            }
        }

        direction = MOTOR_DIRECTION.LEFT;
    }

    public void moveRight(boolean found, boolean forced) {
        if (direction != MOTOR_DIRECTION.RIGHT) {
            lastReqTime = new Date();
            bb8Api.moveRight(found);
        } else {
            Date now = new Date();
            if (forced) {
                lastReqTime = now;
                bb8Api.moveRight(found);
            } else {
                long lostMs = (now.getTime() - lastReqTime.getTime());
                if (lostMs > 1000) {
                    lastReqTime = now;
                    bb8Api.moveRight(found);
                }
            }
        }

        direction = MOTOR_DIRECTION.RIGHT;
    }

    public void stopNow(boolean found) {
        if (direction != MOTOR_DIRECTION.NONE) {
            lastReqTime = new Date();
            bb8Api.stopNow(found);
        } else {
            Date now = new Date();
            long lostMs = (now.getTime() - lastReqTime.getTime());
            if (lostMs > 1000) {
                lastReqTime = now;
                bb8Api.stopNow(found);
            }
        }

        direction = MOTOR_DIRECTION.NONE;
    }
}
