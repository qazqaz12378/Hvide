package com.example.hyc.ffmpegtest;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener{

    private DNPlayer dnPlayer;
    private SurfaceView surfaceView;
    private int height = 0;
    private int curSystemUiVisibility = 0;
    private SeekBar seekBar;
    private TextView curTime;
    private TextView totalTime;
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
        seekBar = findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(this);
        curTime = findViewById(R.id.currentTime);
        totalTime = findViewById(R.id.totalTime);
        dnPlayer.setOnPrepareListener(new DNPlayer.OnPrepareListener() {
            @Override
            public void onPorepare() {
                final int duration = dnPlayer.GetDUration();

                if(duration != 0){
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            seekBar.setVisibility(View.VISIBLE);
                            curTime.setVisibility(View.VISIBLE);
                            totalTime.setVisibility(View.VISIBLE);
                        }
                    });
                }
                dnPlayer.start();
            }
        });
        dnPlayer.setOnProgressListener(new DNPlayer.OnProgressListener() {
            @Override
            public void onProgress(final int progress) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        int duration = dnPlayer.GetDUration();
                        if(duration !=0){
                          seekBar.setProgress(progress * 100 / duration);
                          curTime.setText(timeMParse(progress));
                          totalTime.setText(timeMParse(duration));
                        }
                    }
                });
            }
        });
      //  dnPlayer.setDataSource("/sdcard/dong.mp4");
        dnPlayer.setDataSource("http://101.44.1.126/mp4files/823300000775AEAC/vip.zuiku8.com/1810/%E9%BB%84%E9%87%91%E5%85%84%E5%BC%9F.HD1280%E9%AB%98%E6%B8%85%E5%9B%BD%E8%AF%AD%E4%B8%AD%E5%AD%97%E7%89%88.mp4");
        requestAllPower();
        surfaceView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                start();
            }
        });
    }

    public void start(View view) {
        dnPlayer.prepare();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            hideBottomUIMenu(true);
            keepScreenLongLight(this,true);
            //横屏
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager
                    .LayoutParams.FLAG_FULLSCREEN);
            ViewGroup.LayoutParams m = surfaceView.getLayoutParams();
            m.width = m.MATCH_PARENT;
            m.height = m.MATCH_PARENT;
            surfaceView.setLayoutParams(m);
        } else {
            hideBottomUIMenu(false);
            keepScreenLongLight(this,false);
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
        } else {
         //   start();
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
                      //  start();
                    }
                } else {
                    Toast.makeText(this, "" + "权限" + permissions[i] + "申请失败", Toast.LENGTH_SHORT).show();
                }
            }
        }
    }

    //endregion
// region 隐藏虚拟按键 并且全屏 true:隐藏   false:显示
    protected void hideBottomUIMenu(boolean isHideMenu) {
        if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) {
            //低版本的
            View v = this.getWindow().getDecorView();
            if (isHideMenu) {
                v.setSystemUiVisibility(View.GONE);
            } else {
                v.setSystemUiVisibility(0);
            }
        } else if (Build.VERSION.SDK_INT >= 19) {
            View decorView = getWindow().getDecorView();
            if (curSystemUiVisibility == 0)
                curSystemUiVisibility = decorView.getSystemUiVisibility();
            if (isHideMenu) {

                int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN;
                decorView.setSystemUiVisibility(uiOptions);
            } else {
                decorView.setSystemUiVisibility(0);
            }
        }
    }

    //endregion

    //region 屏幕是否常量 true:常量
    protected static void keepScreenLongLight(Activity activity, boolean isOpenLight) {
        Window window = activity.getWindow();
        if (isOpenLight) {
            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } else {
            window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    //endregion
    public void start() {
        dnPlayer.prepare();
    }

    @Override
    protected void onStop() {
        super.onStop();
        dnPlayer.stop();
    }

    @Override
    protected void onResume() {
        super.onResume();
        dnPlayer.onResume();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        dnPlayer.release();
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }
    /**
     * Android 音乐播放器应用里，读出的音乐时长为 long 类型以毫秒数为单位，例如：将 234736 转化为分钟和秒应为 03:55 （包含四舍五入）
     * @param duration 音乐时长
     * @return
     */
    public static String timeParse(long duration) {
        String time = "" ;
        long minute = duration / 60000;
        long seconds = duration % 60000;
        long second = Math.round((float)seconds/1000) ;
        if( minute < 10 ){
            time += "0" ;
        }
        time += minute+":" ;
        if( second < 10 ){
            time += "0" ;
        }
        time += second ;
        return time ;
    }
    /**
     * Android 音乐播放器应用里，读出的音乐时长为 long 类型以毫秒数为单位，例如：将 234736 转化为分钟和秒应为 03:55 （包含四舍五入）
     * @param duration 音乐时长   秒转换
     * @return
     */
    public static String timeMParse(long duration) {
        String time = "" ;
        long minute = duration / 60;
        long seconds = duration % 60;
        long second = Math.round((float)seconds) ;
        if( minute < 10 ){
            time += "0" ;
        }
        time += minute+":" ;
        if( second < 10 ){
            time += "0" ;
        }
        time += second ;
        return time ;
    }

}
