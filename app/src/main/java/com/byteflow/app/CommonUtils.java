package com.byteflow.app;

import android.content.Context;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class CommonUtils {
    private static final String TAG = "CommonUtils";

    public static void copyAssetsDirToSDCard(Context context, String assetsDirName, String sdCardPath) {
        Log.d(TAG, "copyAssetsDirToSDCard() called with: context = [" + context + "], assetsPath = [" + assetsDirName + "], sdCardPath = [" + sdCardPath + "]");
        try {
            String list[] = context.getAssets().list(assetsDirName);
            if (list.length == 0) { // 说明assetsPath为空,或者assetsPath是一个文件
                InputStream mIs = context.getAssets().open(assetsDirName); // 读取流
                byte[] mByte = new byte[1024];
                int bt = 0;
                File file = new File(sdCardPath + File.separator
                        + assetsDirName.substring(assetsDirName.lastIndexOf('/')));
                if (!file.exists()) {
                    file.createNewFile(); // 创建文件
                } else {
                    return;//已经存在直接退出
                }
                FileOutputStream fos = new FileOutputStream(file); // 写入流
                while ((bt = mIs.read(mByte)) != -1) { // assets为文件,从文件中读取流
                    fos.write(mByte, 0, bt);// 写入流到文件中
                }
                fos.flush();// 刷新缓冲区
                mIs.close();// 关闭读取流
                fos.close();// 关闭写入流
            } else { // 当mString长度大于0,说明其为文件夹
                String subDirName = assetsDirName;
                if(assetsDirName.contains("/")) {
                    subDirName = assetsDirName.substring(assetsDirName.lastIndexOf('/')+1);
                }
                sdCardPath = sdCardPath + File.separator + subDirName;
                File file = new File(sdCardPath);
                if (!file.exists())
                    file.mkdirs(); // 在sd下创建目录
                for (String stringFile : list) { // 进行递归
                    copyAssetsDirToSDCard(context, assetsDirName + File.separator + stringFile, sdCardPath);
                }
            }
        } catch (
                Exception e) {
            e.printStackTrace();
        }

    }
}
