package com.example.hyc.ffmpegtest;

import android.content.res.Configuration;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private DNPlayer dnPlayer;
    private  SurfaceView surfaceView;
    private int height = 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = findViewById(R.id.surfaceView);
        ViewGroup.LayoutParams m = surfaceView.getLayoutParams();
        final float scale = this.getResources().getDisplayMetrics().density;
        height = (int)(200 * scale + 0.5f);
        dnPlayer = new DNPlayer();
        dnPlayer.setSurfaceView(surfaceView);
        dnPlayer.setDataSource("/sdcard/dong.mp4");
        dnPlayer.setOnPrepareListener(new DNPlayer.OnPrepareListener() {
            @Override
            public void onPorepare() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this,"可以开始播放了",Toast.LENGTH_LONG).show();
                    }
                });
                dnPlayer.start();
            }
        });
        dnPlayer.prepare();
    }
    public void start(View view){
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
}
