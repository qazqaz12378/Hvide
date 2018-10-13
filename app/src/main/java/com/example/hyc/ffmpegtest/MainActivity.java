package com.example.hyc.ffmpegtest;

import android.Manifest;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private DNPlayer dnPlayer;
    private SurfaceView surfaceView;
    private int height = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = findViewById(R.id.surfaceView);
        ViewGroup.LayoutParams m = surfaceView.getLayoutParams();
        final float scale = this.getResources().getDisplayMetrics().density;
        height = (int) (200 * scale + 0.5f);
        dnPlayer = new DNPlayer();
        dnPlayer.setSurfaceView(surfaceView);

        dnPlayer.setDataSource("/sdcard/dong.mp4");
        dnPlayer.setOnPrepareListener(new DNPlayer.OnPrepareListener() {
            @Override
            public void onPorepare() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "可以开始播放了", Toast.LENGTH_LONG).show();
                    }
                });
                dnPlayer.start();
            }
        });
        requestAllPower();
    }

    public void start(View view) {
        dnPlayer.prepare();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            //横屏
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager
                    .LayoutParams.FLAG_FULLSCREEN);
            ViewGroup.LayoutParams m = surfaceView.getLayoutParams();
            m.width = m.MATCH_PARENT;
            m.height = m.MATCH_PARENT;
            surfaceView.setLayoutParams(m);
        } else {
            //竖屏
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            ViewGroup.LayoutParams m = surfaceView.getLayoutParams();
            m.width = m.MATCH_PARENT;
            m.height = height;
            surfaceView.setLayoutParams(m);
        }

    }
//region 动态申请权限
    public void requestAllPower() {
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            } else {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
                                Manifest.permission.READ_EXTERNAL_STORAGE}, 1);
            }
        }else
        {
            start();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 1) {
            for (int i = 0; i < permissions.length; i++) {
                if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(this, "" + "权限" + permissions[i] + "申请成功", Toast.LENGTH_SHORT).show();
                    if (i > 0) {
                        start();
                    }
                } else {
                    Toast.makeText(this, "" + "权限" + permissions[i] + "申请失败", Toast.LENGTH_SHORT).show();
                }
            }
        }
    }
//endregion
    public void start() {
        dnPlayer.prepare();
    }
}
