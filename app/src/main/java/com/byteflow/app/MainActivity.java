package com.byteflow.app;

import android.Manifest;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

import com.byteflow.app.adapter.MyRecyclerViewAdapter;
import com.byteflow.app.audio.AudioCollector;
import com.byteflow.app.egl.EGLActivity;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;

import static android.opengl.GLSurfaceView.RENDERMODE_CONTINUOUSLY;
import static android.opengl.GLSurfaceView.RENDERMODE_WHEN_DIRTY;
import static com.byteflow.app.MyGLSurfaceView.IMAGE_FORMAT_NV21;
import static com.byteflow.app.MyGLSurfaceView.IMAGE_FORMAT_RGBA;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_BASIC_LIGHTING;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_BLENDING;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_COORD_SYSTEM;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_DEPTH_TESTING;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_EGL;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_FBO;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_FBO_LEG;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_INSTANCING;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_BEATING_HEART;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_BEZIER_CURVE;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_BIG_EYES;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_BIG_HEAD;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_CLOUD;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_FACE_SLENDER;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_ROTARY_HEAD;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_SHOCK_WAVE;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_KEY_VISUALIZE_AUDIO;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_MULTI_LIGHTS;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_PARTICLES;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_PBO;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_SKYBOX;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_STENCIL_TESTING;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_TEXTURE_MAP;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_TRANS_FEEDBACK;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_TRIANGLE;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_VAO;
import static com.byteflow.app.MyNativeRender.SAMPLE_TYPE_YUV_TEXTURE_MAP;

public class MainActivity extends AppCompatActivity implements AudioCollector.Callback{
    private static final String TAG = "MainActivity";
    private static final String[] REQUEST_PERMISSIONS = {
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.RECORD_AUDIO,
    };
    private static final int PERMISSION_REQUEST_CODE = 1;
    private static final String[] SAMPLE_TITLES = {
            "DrawTriangle",
            "TextureMap",
            "YUV Rendering",
            "VAO&VBO",
            "FBO Offscreen Rendering",
            "EGL Background Rendering",
            "FBO Stretching",
            "Coordinate System",
            "Basic Lighting",
            "Transform Feedback",
            "Complex Lighting",
            "Depth Testing",
            "Instancing",
            "Stencil Testing",
            "Blending",
            "Particles",
            "SkyBox",
            "Assimp Load 3D Model",
            "PBO",
            "Beating Heart",
            "Cloud",
            "Shock Wave",
            "Bezier Curve",
            "Big Eyes",
            "Face Slender",
            "Big Head",
            "Rotary Head",
            "Visualize Audio"
    };

    private MyGLSurfaceView mGLSurfaceView;
    private ViewGroup mRootView;
    private int mSampleSelectedIndex = SAMPLE_TYPE_KEY_BEATING_HEART - SAMPLE_TYPE;
    private AudioCollector mAudioCollector;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mRootView = (ViewGroup) findViewById(R.id.rootView);
        mGLSurfaceView = (MyGLSurfaceView) findViewById(R.id.my_gl_surface_view);
        mGLSurfaceView.getGLRender().init();
        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
            ActivityCompat.requestPermissions(this, REQUEST_PERMISSIONS, PERMISSION_REQUEST_CODE);
        }
        CommonUtils.copyAssetsDirToSDCard(MainActivity.this, "poly", "/sdcard/model");
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == PERMISSION_REQUEST_CODE) {
            if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
                Toast.makeText(this, "We need the permission: WRITE_EXTERNAL_STORAGE", Toast.LENGTH_SHORT).show();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mAudioCollector != null) {
            mAudioCollector.uninit();
            mAudioCollector = null;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mGLSurfaceView.getGLRender().unInit();
        /*
        * Once the EGL context gets destroyed all the GL buffers etc will get destroyed with it,
        * so this is unnecessary.
        * */
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_change_sample) {
            showGLSampleDialog();
        }
        return true;
    }

    @Override
    public void onAudioBufferCallback(short[] buffer) {
        Log.e(TAG, "onAudioBufferCallback() called with: buffer[0] = [" + buffer[0] + "]");
        mGLSurfaceView.getGLRender().setAudioData(buffer);
        mGLSurfaceView.requestRender();
    }

    private void showGLSampleDialog() {
        final AlertDialog.Builder builder = new AlertDialog.Builder(this);
        LayoutInflater inflater = LayoutInflater.from(this);
        final View rootView = inflater.inflate(R.layout.sample_selected_layout, null);

        final AlertDialog dialog = builder.create();

        Button confirmBtn = rootView.findViewById(R.id.confirm_btn);
        confirmBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.cancel();
            }
        });

        final RecyclerView resolutionsListView = rootView.findViewById(R.id.resolution_list_view);

        final MyRecyclerViewAdapter myPreviewSizeViewAdapter = new MyRecyclerViewAdapter(this, Arrays.asList(SAMPLE_TITLES));
        myPreviewSizeViewAdapter.setSelectIndex(mSampleSelectedIndex);
        myPreviewSizeViewAdapter.addOnItemClickListener(new MyRecyclerViewAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(View view, int position) {
                int selectIndex = myPreviewSizeViewAdapter.getSelectIndex();
                myPreviewSizeViewAdapter.setSelectIndex(position);
                myPreviewSizeViewAdapter.notifyItemChanged(selectIndex);
                myPreviewSizeViewAdapter.notifyItemChanged(position);
                mSampleSelectedIndex = position;
                mGLSurfaceView.setRenderMode(RENDERMODE_WHEN_DIRTY);

                if (mRootView.getWidth() != mGLSurfaceView.getWidth()
                        || mRootView.getHeight() != mGLSurfaceView.getHeight()) {
                    mGLSurfaceView.setAspectRatio(mRootView.getWidth(), mRootView.getHeight());
                }

                mGLSurfaceView.getGLRender().setParamsInt(SAMPLE_TYPE, position + SAMPLE_TYPE, 0);

                int sampleType = position + SAMPLE_TYPE;

                switch (sampleType) {
                    case SAMPLE_TYPE_TRIANGLE:
                        break;
                    case SAMPLE_TYPE_TEXTURE_MAP:
                        loadRGBAImage(R.drawable.dzzz);
                        break;
                    case SAMPLE_TYPE_YUV_TEXTURE_MAP:
                        loadNV21Image();
                        break;
                    case SAMPLE_TYPE_VAO:
                        break;
                    case SAMPLE_TYPE_FBO:
                        loadRGBAImage(R.drawable.java);
                        break;
                    case SAMPLE_TYPE_FBO_LEG:
                        loadRGBAImage(R.drawable.leg);
                        break;
                    case SAMPLE_TYPE_EGL:
                        startActivity(new Intent(MainActivity.this, EGLActivity.class));
                        break;
                    case SAMPLE_TYPE_COORD_SYSTEM:
                    case SAMPLE_TYPE_BASIC_LIGHTING:
                    case SAMPLE_TYPE_TRANS_FEEDBACK:
                    case SAMPLE_TYPE_MULTI_LIGHTS:
                    case SAMPLE_TYPE_DEPTH_TESTING:
                    case SAMPLE_TYPE_INSTANCING:
                    case SAMPLE_TYPE_STENCIL_TESTING:
                        loadRGBAImage(R.drawable.board_texture);
                        break;
                    case SAMPLE_TYPE_BLENDING:
                        loadRGBAImage(R.drawable.board_texture,0);
                        loadRGBAImage(R.drawable.floor,1);
                        loadRGBAImage(R.drawable.window,2);
                        break;
                    case SAMPLE_TYPE_PARTICLES:
                        //mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        loadRGBAImage(R.drawable.board_texture);
                        break;
                    case SAMPLE_TYPE_SKYBOX:
                        loadRGBAImage(R.drawable.right,0);
                        loadRGBAImage(R.drawable.left,1);
                        loadRGBAImage(R.drawable.top,2);
                        loadRGBAImage(R.drawable.bottom,3);
                        loadRGBAImage(R.drawable.back,4);
                        loadRGBAImage(R.drawable.front,5);
                        break;
                    case SAMPLE_TYPE_PBO:
                        loadRGBAImage(R.drawable.front);
                        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        break;
                    case SAMPLE_TYPE_KEY_BEATING_HEART:
                        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        break;
                    case SAMPLE_TYPE_KEY_CLOUD:
                        loadRGBAImage(R.drawable.noise);
                        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        break;
                    case SAMPLE_TYPE_KEY_SHOCK_WAVE:
                        loadRGBAImage(R.drawable.front);
                        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        break;
                    case SAMPLE_TYPE_KEY_BEZIER_CURVE:
                        //loadRGBAImage(R.drawable.board_texture);
                        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        break;
                    case SAMPLE_TYPE_KEY_BIG_EYES:
                    case SAMPLE_TYPE_KEY_FACE_SLENDER:
                        Bitmap bitmap = loadRGBAImage(R.drawable.yifei);
                        mGLSurfaceView.setAspectRatio(bitmap.getWidth(), bitmap.getHeight());
                        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        break;
                    case SAMPLE_TYPE_KEY_BIG_HEAD:
                    case SAMPLE_TYPE_KEY_ROTARY_HEAD:
                        Bitmap b = loadRGBAImage(R.drawable.huge);
                        mGLSurfaceView.setAspectRatio(b.getWidth(), b.getHeight());
                        mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                        break;
                    case SAMPLE_TYPE_KEY_VISUALIZE_AUDIO:
                        if(mAudioCollector == null) {
                            mAudioCollector = new AudioCollector();
                            mAudioCollector.addCallback(MainActivity.this);
                            mAudioCollector.init();
                        }
                        //mGLSurfaceView.setRenderMode(RENDERMODE_CONTINUOUSLY);
                    default:
                        break;
                }

                mGLSurfaceView.requestRender();

                if(sampleType != SAMPLE_TYPE_KEY_VISUALIZE_AUDIO && mAudioCollector != null) {
                    mAudioCollector.uninit();
                    mAudioCollector = null;
                }

                dialog.cancel();
            }
        });

        LinearLayoutManager manager = new LinearLayoutManager(this);
        manager.setOrientation(LinearLayoutManager.VERTICAL);
        resolutionsListView.setLayoutManager(manager);

        resolutionsListView.setAdapter(myPreviewSizeViewAdapter);
        resolutionsListView.scrollToPosition(mSampleSelectedIndex);

        dialog.show();
        dialog.getWindow().setContentView(rootView);

    }

    private Bitmap loadRGBAImage(int resId) {
        InputStream is = this.getResources().openRawResource(resId);
        Bitmap bitmap;
        try {
            bitmap = BitmapFactory.decodeStream(is);
            if (bitmap != null) {
                int bytes = bitmap.getByteCount();
                ByteBuffer buf = ByteBuffer.allocate(bytes);
                bitmap.copyPixelsToBuffer(buf);
                byte[] byteArray = buf.array();
                mGLSurfaceView.getGLRender().setImageData(IMAGE_FORMAT_RGBA, bitmap.getWidth(), bitmap.getHeight(), byteArray);
            }
        }
        finally
        {
            try
            {
                is.close();
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }
        return bitmap;
    }

    private void loadRGBAImage(int resId, int index) {
        InputStream is = this.getResources().openRawResource(resId);
        Bitmap bitmap;
        try {
            bitmap = BitmapFactory.decodeStream(is);
            if (bitmap != null) {
                int bytes = bitmap.getByteCount();
                ByteBuffer buf = ByteBuffer.allocate(bytes);
                bitmap.copyPixelsToBuffer(buf);
                byte[] byteArray = buf.array();
                mGLSurfaceView.getGLRender().setImageDataWithIndex(index, IMAGE_FORMAT_RGBA, bitmap.getWidth(), bitmap.getHeight(), byteArray);
            }
        }
        finally
        {
            try
            {
                is.close();
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    private void loadNV21Image() {
        InputStream is = null;
        try {
            is = getAssets().open("YUV_Image_840x1074.NV21");
        } catch (IOException e) {
            e.printStackTrace();
        }

        int lenght = 0;
        try {
            lenght = is.available();
            byte[] buffer = new byte[lenght];
            is.read(buffer);
            mGLSurfaceView.getGLRender().setImageData(IMAGE_FORMAT_NV21, 840, 1074, buffer);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try
            {
                is.close();
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }

    }

    protected boolean hasPermissionsGranted(String[] permissions) {
        for (String permission : permissions) {
            if (ActivityCompat.checkSelfPermission(this, permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }
}
