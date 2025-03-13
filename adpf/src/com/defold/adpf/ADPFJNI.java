package com.defold.adpf;

import android.os.PerformanceHintManager;
import android.os.PerformanceHintManager.Session;
import android.os.PowerManager;
import android.content.Context;
import android.app.Activity;
import android.util.Log;

public class ADPFJNI {
    private static final String TAG = "ADPFJNI";

    private Activity activity;
    private PerformanceHintManager.Session hintSession;
    private PowerManager powerManager;
    private long timestamp;

    public ADPFJNI(Activity activity) {
        this.activity = activity;
    }

    public int hintInitialize(long targetFpsNanos) {
        if (this.hintSession != null) {
            Log.e(TAG, "hintInitialize() - Already initialized");
            return 1;
        }

        int[] tids = {
            android.os.Process.myTid()
        };
        Log.d(TAG, "hintInitialize() target fps: " + targetFpsNanos + "ns tid: " + tids[0]);
        PerformanceHintManager performanceHintManager = (PerformanceHintManager)this.activity.getSystemService(Context.PERFORMANCE_HINT_SERVICE);
        this.hintSession = performanceHintManager.createHintSession(tids, targetFpsNanos);
        this.timestamp = System.nanoTime();

        if (this.hintSession != null) {
            Log.d(TAG, "hintInitialize() success!");
            return 1;
        }
        else {
            Log.d(TAG, "hintInitialize() failed!");
            return 0;
        }
    }

    public void hintFinalize() {
        if (this.hintSession != null) {
            Log.d(TAG, "hintFinalize()");
            this.hintSession.close();
            this.hintSession = null;
        }
    }

    public void hintReportWorkDuration() {
        if (this.hintSession == null) {
            Log.e(TAG, "hintReportWorkDuration() - Not initialized");
            return;
        }
        long now = System.nanoTime();
        long duration = now - this.timestamp;
        this.hintSession.reportActualWorkDuration(duration);
        this.timestamp = now;
    }


    public void hintUpdateTargetFPS(long targetFpsNanos) {
        if (this.hintSession == null) {
            Log.e(TAG, "hintUpdateTargetFPS() - Not initialized");
            return;
        }
        Log.d(TAG, "hintUpdateTargetFPS()");
        this.hintSession.updateTargetWorkDuration(targetFpsNanos);
    }


    public int thermalInitialize() {
        if (this.powerManager != null) {
            Log.e(TAG, "thermalInitialize() - Already initialized");
            return 1;
        }

        this.powerManager = (PowerManager)this.activity.getSystemService(Context.POWER_SERVICE);
        if (this.powerManager == null) {
            Log.d(TAG, "thermalInitialize() failed!");
            return 0;
        }

        float headroom = this.powerManager.getThermalHeadroom(1);
        if (headroom == 0.0 || Float.isNaN(headroom)) {
            Log.d(TAG, "thermalInitialize() failed!");
            this.powerManager = null;
            return 0;
        }

        Log.d(TAG, "thermalInitialize() success!");
        return 1;
    }

    public void thermalFinalize() {
        if (this.powerManager != null) {
            Log.d(TAG, "thermalFinalize()");
            this.powerManager = null;
        }
    }

    public float thermalGetHeadroom(int forecastSeconds) {
        if (this.powerManager == null) {
            Log.e(TAG, "thermalGetHeadroom() - Not initialized");
            return -1;
        }
        Log.d(TAG, "thermalGetHeadroom()");
        return this.powerManager.getThermalHeadroom(forecastSeconds);
    }

    public int thermalGetStatus() {
        if (this.powerManager == null) {
            Log.e(TAG, "thermalGetStatus() - Not initialized");
            return -1;
        }
        Log.d(TAG, "thermalGetStatus()");
        return this.powerManager.getCurrentThermalStatus();
    }

}
