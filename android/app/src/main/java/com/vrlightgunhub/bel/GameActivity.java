// SPDX-License-Identifier: GPL-2.0-or-later
package com.vrlightgunhub.bel;
import org.libsdl.app.SDLActivity;
import android.os.Environment;
import android.content.Intent;
import android.os.Build;
import android.Manifest;
import android.content.pm.PackageManager;
import java.io.File;

public final class GameActivity extends SDLActivity {
    @Override protected void onNativeMainReturned() {
        Intent setup=new Intent(Intent.ACTION_MAIN).setClass(this,SetupActivity.class).putExtra("returned",true);
        setup.addCategory("com.oculus.intent.category.2D");
        setup.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(setup); finishAndRemoveTask();
    }
    @Override protected String[] getLibraries() { return new String[]{"SDL2","main"}; }
    @Override protected String[] getArguments() {
        String root=getFilesDir().getAbsolutePath();
        File download=Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        File external=getExternalFilesDir(null);
        File owned=external!=null?external:getFilesDir();
        boolean downloadsAllowed=Build.VERSION.SDK_INT>=30?Environment.isExternalStorageManager():
            checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)==PackageManager.PERMISSION_GRANTED;
        File games=GameStorage.romDirectory(owned,download,downloadsAllowed);
        nativeSetenv("BEL_VR","1");
        // Validated GPU-world presentation skips duplicate CPU 3D work only behind native safety guards.
        nativeSetenv("BEL_FAST_RASTER","1");
        nativeSetenv("BEL_UART_COALESCE","0");
        nativeSetenv("BEL_XR_LIBRARY",getApplicationInfo().nativeLibraryDir+"/libbel_xr.so");
        // The worker adapts playback to measured native production without
        // changing the arcade clocks, synthesized audio, or reference capture.
        nativeSetenv("BEL_AUDIO_LIBRARY",getApplicationInfo().nativeLibraryDir+"/libbel_audio.so");
        nativeSetenv("BEL_AUDIO_STRETCH","0");
        nativeSetenv("BEL_ASSETS",root+"/assets");
        nativeSetenv("BEL_SETTINGS",root+"/settings.ini");
        File logs=getExternalFilesDir(null);
        nativeSetenv("BEL_XR_LOG",new File(logs!=null?logs:getFilesDir(),"bel-xr.log").getAbsolutePath());
        nativeSetenv("BEL_RASTER_LOG",new File(logs!=null?logs:getFilesDir(),"bel-raster.tsv").getAbsolutePath());
        nativeSetenv("BEL_AUDIO_LOG",new File(logs!=null?logs:getFilesDir(),"bel-audio.tsv").getAbsolutePath());
        nativeSetenv("BEL_CUSTOM_GUN",GameStorage.customGun(owned,download,downloadsAllowed).getAbsolutePath());
        return new String[]{"bel","-rompath",games.getAbsolutePath(),
            "-nvram_directory",root+"/nvram","-cfg_directory",root+"/cfg","-state_directory",root+"/state","-snapshot_directory",root+"/snap",
            "-video","none","-sound","sdl","-skip_gameinfo","-noreadconfig","-nomouse","-numprocessors","2","-throttle"};
    }
}
