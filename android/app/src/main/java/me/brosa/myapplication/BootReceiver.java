package me.brosa.myapplication;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class BootReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Intent notificationListener = new Intent(context, NotificationListenerService.class);
        context.startService(notificationListener);
    }
}
