package org.lineageos.oplus;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.Context;

import android.util.Log;

import org.lineageos.oplus.disablehwoverlays.DisableHWOverlaysService;
import org.lineageos.oplus.aodhelper.DozeCustomService;

public class OnLockedBootCompleteReceiver extends BroadcastReceiver {
    private static final String TAG = "OplusDeviceService";

    Intent sIntent;

    @Override
    public void onReceive(final Context context, Intent intent) {
        Log.i(TAG, "onBoot");

        sIntent = new Intent(context, DisableHWOverlaysService.class);
        context.startService(sIntent);

        sIntent = new Intent(context, DozeCustomService.class);
        context.startService(sIntent);
    }
}
