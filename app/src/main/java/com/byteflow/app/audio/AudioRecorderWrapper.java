package com.byteflow.app.audio;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

public class AudioRecorderWrapper implements AudioRecord.OnRecordPositionUpdateListener{
    private static final String TAG = "AudioRecorderWrapper";
    private static final int RECORDER_SAMPLE_RATE = 44100;
    private static final int RECORDER_CHANNELS = 1;
    private static final int RECORDER_ENCODING_BIT = 16;
    private static final int RECORDER_AUDIO_ENCODING = AudioFormat.ENCODING_PCM_16BIT;
    private AudioRecord mAudioRecord;
    private Thread mThread;
    private byte[] mAudioBuffer;
    private Handler mHandler;
    private int mBufferSize;
    private Callback mCallback;

    public AudioRecorderWrapper() {
        mBufferSize = 2 * AudioRecord.getMinBufferSize(RECORDER_SAMPLE_RATE,
                RECORDER_CHANNELS, RECORDER_AUDIO_ENCODING);
        mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, RECORDER_SAMPLE_RATE,
                RECORDER_CHANNELS, RECORDER_AUDIO_ENCODING, mBufferSize);
    }

    public void initWrapper() {
        mAudioRecord.startRecording();
        mThread = new Thread("Audio-Recorder") {
            @Override
            public void run() {
                super.run();
                mAudioBuffer = new byte[mBufferSize];
                Looper.prepare();
                mHandler = new Handler(Looper.myLooper());
                mAudioRecord.setRecordPositionUpdateListener(AudioRecorderWrapper.this, mHandler);
                int bytePerSample = RECORDER_ENCODING_BIT / 8;
                float samplesToDraw = mBufferSize / bytePerSample;
                mAudioRecord.setPositionNotificationPeriod((int) samplesToDraw);
                mAudioRecord.read(mAudioBuffer, 0, mBufferSize);
                Looper.loop();
            }
        };
        mThread.start();
    }

    public void uninitWrapper() {
        mAudioRecord.release();
        mHandler.getLooper().quitSafely();
    }

    public void addCallback(Callback callback) {
        mCallback = callback;
    }

    @Override
    public void onMarkerReached(AudioRecord recorder) {

    }

    @Override
    public void onPeriodicNotification(AudioRecord recorder) {
        if (mAudioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING
                && mAudioRecord.read(mAudioBuffer, 0, mAudioBuffer.length) != -1)
        {
            if(mCallback != null)
                mCallback.onAudioBufferCallback(mAudioBuffer);
        }

    }


    interface Callback {
        void onAudioBufferCallback(byte[] buffer); //little-endian
    }
}
