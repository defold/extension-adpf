package com.defold.adpf;

import android.os.PerformanceHintManager;
import android.os.PerformanceHintManager.Session;
import android.content.Context;
import android.app.Activity;
import android.util.Log;

public class ADPFJNI {
    private static final String TAG = "ADPFJNI";

    private Activity activity;
    private PerformanceHintManager.Session hintSession;
    private long timestamp;

    public ADPFJNI(Activity activity) {
        this.activity = activity;
    }

    public void initialize(long targetFpsNanos) {
        if (this.hintSession != null) {
            Log.e(TAG, "initialize() - Already initialized");
            return;
        }

        int[] tids = {
            android.os.Process.myTid()
        };
        Log.d(TAG, "initialize() target fps: " + targetFpsNanos + "ns tid: " + tids[0]);
        PerformanceHintManager performanceHintManager = (PerformanceHintManager)this.activity.getSystemService(Context.PERFORMANCE_HINT_SERVICE);
        this.hintSession = performanceHintManager.createHintSession(tids, targetFpsNanos);
        this.timestamp = System.nanoTime();

        if (this.hintSession != null) {
            Log.d(TAG, "initialize() success!");
        }
        else {
            Log.d(TAG, "initialize() failed!");
        }
    }

    public void finalize() {
        if (this.hintSession != null) {
            Log.d(TAG, "finalize()");
            this.hintSession.close();
            this.hintSession = null;
        }
    }

    public void reportWorkDuration() {
        if (this.hintSession == null) {
            Log.e(TAG, "reportWorkDuration() - Not initialized");
            return;
        }
        long now = System.nanoTime();
        long duration = now - this.timestamp;
        this.hintSession.reportActualWorkDuration(duration);
        this.timestamp = now;
    }


    public void updateTargetFPS(long targetFpsNanos) {
        if (this.hintSession == null) {
            Log.e(TAG, "updateTargetFPS() - Not initialized");
            return;
        }
        Log.d(TAG, "updateTargetFPS()");
        this.hintSession.updateTargetWorkDuration(targetFpsNanos);
    }
}
