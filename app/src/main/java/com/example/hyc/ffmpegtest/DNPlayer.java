package com.example.hyc.ffmpegtest;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * 提供java 进行播放 停止函数
 */
public class DNPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    private String dataSource;
    private SurfaceHolder holder;
    private OnPrepareListener listener;
    /**
     * 让使用设置播放的文件或者直播地址
     */
    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }
    public void setSurfaceView(SurfaceView surfaceView){
        holder = surfaceView.getHolder();
        holder.addCallback(this);
    }
    public void onError(int errorCode){

    }
    public void onPrepare(){
        if(null != listener){
            listener.onPorepare();
        }
    }
    public void setOnPrepareListener(OnPrepareListener listener){
        this.listener = listener;
    }
    public interface OnPrepareListener{
        void onPorepare();
    }
    public void prepare(){
        native_perpare(dataSource);
    }
    public void start() {
        native_start();
    }

    public void stop() {
    }
    public void release(){
        holder.removeCallback(this);
    }
    /**
     * 画布创建好了
     * @param surfaceHolder
     */
    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        native_setSurface(surfaceHolder.getSurface());
    }

    /**
     * 画布发生了变化（横竖屏切换，按了home都会会掉这个函数）
     * @param surfaceHolder
     * @param i
     * @param i1
     * @param i2
     */
    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    /**
     * 销毁画布（按了home/退出应用）
     * @param surfaceHolder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }
    native void native_perpare(String dataSource);
    native void native_start();
    native void native_setSurface(Surface surface);
}
