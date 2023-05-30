// Copyright 2021 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.painnick.bb8;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

//import com.google.mediapipe.examples.facedetection.R;
import com.google.mediapipe.formats.proto.DetectionProto;
import com.google.mediapipe.formats.proto.LocationDataProto;
import com.google.mediapipe.solutions.facedetection.FaceDetection;
import com.google.mediapipe.solutions.facedetection.FaceDetectionOptions;

import java.util.Date;

enum FACE_DIRECTION {
    NONE,
    LEFT,
    RIGHT
}

/**
 * Main activity of MediaPipe Face Detection app.
 */
public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";

    private BB8Controller bb8Controller;
    private FaceDetection faceDetection;

    // Image demo UI and image loader components.
    private FaceDetectionResultImageView imageView;

    private Date lastDetection, startFoundSeq, startNotFoundSeq;
    private FACE_DIRECTION faceDirection = FACE_DIRECTION.NONE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        lastDetection = new Date();
        startFoundSeq = null;
        startNotFoundSeq = null;
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);
        setupStaticImageDemoUiComponents();
        Toast.makeText(MainActivity.this, "Wifi를 켜고, BB-8(또는 ESP_XXXXXXXX)에 연결해 주세요.", Toast.LENGTH_SHORT).show();
        bb8Controller.start();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (bb8Controller != null) {
            bb8Controller.resume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (bb8Controller != null) {
            bb8Controller.pause();
        }
    }

    /**
     * Sets up the UI components for the static image demo.
     */
    private void setupStaticImageDemoUiComponents() {
        imageView = new FaceDetectionResultImageView(this);
        setupStaticImageModePipeline();
        bb8Controller = new BB8Controller(bitmap -> faceDetection.send(bitmap), response -> {
        });

        // 버튼과 화면과 위치가 반대임에 유의!
        findViewById(R.id.move_left).setOnClickListener(
                v -> {
                    faceDirection = FACE_DIRECTION.RIGHT;
                    bb8Controller.moveRight(false);
                });
        findViewById(R.id.move_right).setOnClickListener(
                v -> {
                    faceDirection = FACE_DIRECTION.LEFT;
                    bb8Controller.moveLeft(false);
                });
    }

    /**
     * Sets up core workflow for static image mode.
     */
    private void setupStaticImageModePipeline() {
        // Initializes a new MediaPipe Face Detection solution instance in the static image mode.
        faceDetection =
                new FaceDetection(
                        this,
                        FaceDetectionOptions.builder()
                                .setStaticImageMode(true)
                                .setModelSelection(0)
                                .setMinDetectionConfidence(0.7f)
                                .build());

        // Connects MediaPipe Face Detection solution to the user-defined FaceDetectionResultImageView.
        faceDetection.setResultListener(
                faceDetectionResult -> {
                    imageView.setFaceDetectionResult(faceDetectionResult);
                    runOnUiThread(() -> imageView.update());

                    float maxWidth = 0;
                    DetectionProto.Detection foundDetection = null;
                    for (DetectionProto.Detection detection : faceDetectionResult.multiFaceDetections()) {
                        LocationDataProto.LocationData.RelativeBoundingBox box = detection.getLocationData().getRelativeBoundingBox();
                        if (maxWidth < box.getWidth()) {
                            foundDetection = detection;
                            maxWidth = box.getWidth();
                        }
                    }

                    Date now = new Date();
                    long lostSeconds = (now.getTime() - lastDetection.getTime()) / 1000;

                    if (foundDetection != null) { // Found!!!
                        boolean isStartSeq = (startFoundSeq == null);
                        if (isStartSeq) {
                            startFoundSeq = new Date();
                            startNotFoundSeq = null;
                        }

                        // 찾은 지 30초가 지나지 않았다면 굳이 Stop 명령을 보내지 않는다.
                        if (lostSeconds > 30) {
                            bb8Controller.stopNow(true);
                        }

                        lastDetection = new Date();
                    } else {
                        // LOST!!!
                        if ((startNotFoundSeq == null) && (lostSeconds > 2)) {
                            startFoundSeq = null;
                            startNotFoundSeq = new Date();
                        }

                        // 기존 방향으로 카메라 이동
                        if (faceDirection == FACE_DIRECTION.RIGHT) {
                            bb8Controller.moveRight(false);
                        } else {
                            bb8Controller.moveLeft(false);
                        }
                    }
                });
        faceDetection.setErrorListener(
                (message, e) -> Log.e(TAG, "MediaPipe Face Detection error:" + message));

        // Updates the preview layout.
        FrameLayout frameLayout = findViewById(R.id.preview_display_layout);
        frameLayout.removeAllViewsInLayout();
        imageView.setImageDrawable(null);
        frameLayout.addView(imageView);
        imageView.setVisibility(View.VISIBLE);
    }

    private void stopCurrentPipeline() {
        if (faceDetection != null) {
            faceDetection.close();
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    private void hideSystemUI() {
        // Enables regular immersive mode.
        // For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
        // Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE
                        // Set the content to appear under the system bars so that the
                        // content doesn't resize when the system bars hide and show.
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        // Hide the nav bar and status bar
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    // Shows the system bars by removing all the flags
    // except for the ones that make the content appear under the system bars.
    private void showSystemUI() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
    }
}
