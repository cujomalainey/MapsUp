package me.brosa.myapplication;

import android.app.Notification;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.service.notification.StatusBarNotification;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.RemoteViews;
import android.widget.TextView;
import android.widget.Toast;

import java.util.HashSet;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;


public class NotificationListenerService extends android.service.notification.NotificationListenerService {
    public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
    public static String BLE_SHIELD_TX = "713d0003-503e-4c75-ba94-3148f18d941e";
    public static String BLE_SHIELD_RX = "713d0002-503e-4c75-ba94-3148f18d941e";
    public static String BLE_SHIELD_SERVICE = "713d0000-503e-4c75-ba94-3148f18d941e";

    public final static UUID UUID_BLE_SHIELD_TX = UUID
            .fromString(NotificationListenerService.BLE_SHIELD_TX);
    public final static UUID UUID_BLE_SHIELD_RX = UUID
            .fromString(NotificationListenerService.BLE_SHIELD_RX);
    public final static UUID UUID_BLE_SHIELD_SERVICE = UUID
            .fromString(NotificationListenerService.BLE_SHIELD_SERVICE);

    private final String DEBUG_TAG = this.getClass().getSimpleName();

    private BluetoothAdapter mBtAdapter;
    private BluetoothManager mBtManager;
    private BluetoothDevice mBtDevice;
    private BluetoothGatt mBtGatt;
    private BluetoothGattService mBtGattService;

    private String mBtDeviceAddress = null;

    private Set<BluetoothDevice> mDevices = new HashSet<>();

    private Thread mBtScanThread = null;

    private volatile boolean mScanActive = false;
    private boolean mConnected = false;

    private final BluetoothAdapter.LeScanCallback mLeScanCallback = new BluetoothAdapter.LeScanCallback() {
        @Override
        public void onLeScan(BluetoothDevice bluetoothDevice, int i, byte[] bytes) {
            if (!mDevices.contains(bluetoothDevice)) {
                Log.d(DEBUG_TAG, "Discovered " + bluetoothDevice.getName() + " : " + bluetoothDevice.getAddress());
                // Name we agreed on.
                if ("TXRX".equals(bluetoothDevice.getName())) {
                    mBtDevice = bluetoothDevice;
                    connect(bluetoothDevice.getAddress());
                    Log.d(DEBUG_TAG, "Found device TXRX with address: " + mBtDevice.getAddress());
                    stopScan();
                } else {
                    mDevices.add(bluetoothDevice);
                }
            }
        }
    };

    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                mConnected = true;
                mBtGatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d(DEBUG_TAG, "Disconnected from Bluetooth");
                mConnected = false;
                mBtDevice = null;
                scanForDevices();
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d(DEBUG_TAG, "GATT Status success");
                BluetoothGattService service = mBtGatt.getService(NotificationListenerService.UUID_BLE_SHIELD_SERVICE);
                if (service != null) {
                    mBtGattService = service;
                    Log.d(DEBUG_TAG, "Discovered service!");
                }
            } else {
                Log.w(DEBUG_TAG, "onServicesDiscovered received: " + status);
            }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();

        mBtManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBtAdapter = mBtManager.getAdapter();

        if (mBtAdapter == null) {
            Log.e(DEBUG_TAG, "Bluetooth adapter is null, failure.");
            Toast.makeText(getApplicationContext(), "Bluetooth isn't avaialable", Toast.LENGTH_SHORT);
            return;
        } else if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(getApplicationContext(), "BLE not supported", Toast.LENGTH_SHORT);
            return;
        } else if (!mBtAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            getApplicationContext().startActivity(enableBtIntent);
        }

        scanForDevices();
    }

    private boolean connect(final String address) {
        if (mBtDeviceAddress != null && address.equals(mBtDeviceAddress) && mBtGatt != null) {
            return mBtGatt.connect();
        }

        final BluetoothDevice device = mBtAdapter.getRemoteDevice(address);
        if (device == null) {
            Log.d(DEBUG_TAG, "Device " + address + " not found, unable to connect");
            return false;
        }

        mBtGatt = device.connectGatt(this, false, mGattCallback);
        Log.d(DEBUG_TAG, "Trying to create a new connection.");
        mBtDeviceAddress = address;
        mBtDevice = device;

        return true;
    }

    private void scanForDevices() {
        if (mBtScanThread != null) {
            Log.e(DEBUG_TAG, "Scan is currently running, error");
            return;
        }

        mScanActive = true;

        mBtScanThread = new Thread() {
            @Override
            public void run() {
                Log.d(DEBUG_TAG, "Started LE Scan");
                mBtAdapter.startLeScan(null, mLeScanCallback);

                for (long startTime = System.nanoTime() + TimeUnit.SECONDS.toNanos(3); startTime > System.nanoTime();) {
                    if (!mScanActive)
                        break;
                }

                mBtAdapter.stopLeScan(mLeScanCallback);

                if (mBtDevice == null) {
                    run();
                }
            }
        };

        mBtScanThread.start();
    }

    private void stopScan() {
        mScanActive = false;
        mBtScanThread = null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mBtGatt.disconnect();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }

    @Override
    public void onNotificationPosted(StatusBarNotification sbn) {
        Log.d(DEBUG_TAG, "Notification Posted!");
        sendGoogleMapsDirections(sbn);

        super.onNotificationPosted(sbn);
    }

    private void sendGoogleMapsDirections(StatusBarNotification sbn) {
        if (sbn.getPackageName().equals("com.google.android.apps.maps") && !sbn.isClearable()) {
            Notification n = sbn.getNotification();
            RemoteViews rv = n.bigContentView;
            RelativeLayout rl = (RelativeLayout) rv.apply(getApplicationContext(), null);
            LinearLayout ll = (LinearLayout) ((LinearLayout) rl.getChildAt(1)).getChildAt(0);
            TextView distance = (TextView) ll.getChildAt(0);
            TextView direction = (TextView) ll.getChildAt(1);
            TextView eta = (TextView) ll.getChildAt(2);

            Log.d(DEBUG_TAG, distance.getText().toString());
            Log.d(DEBUG_TAG, direction.getText().toString());
            Log.d(DEBUG_TAG, eta.getText().toString());

            if (mConnected && mBtGattService != null) {
                Log.d(DEBUG_TAG, "Sending data");
                BluetoothGattCharacteristic cr = mBtGattService.getCharacteristic(NotificationListenerService.UUID_BLE_SHIELD_TX);

                GoogleMapsDirection.TurnDirection d = null;
                String directionString = direction.getText().toString();

                Pattern pStraight = Pattern.compile("continue|straight|ahead", Pattern.CASE_INSENSITIVE);
                Pattern pLeft = Pattern.compile("left", Pattern.CASE_INSENSITIVE);
                Pattern pRight = Pattern.compile("right", Pattern.CASE_INSENSITIVE);
                Pattern uTurn = Pattern.compile("u-turn", Pattern.CASE_INSENSITIVE);

                if (pStraight.matcher(directionString).matches())
                    d = GoogleMapsDirection.TurnDirection.Straight;
                if (pLeft.matcher(directionString).matches())
                    d = GoogleMapsDirection.TurnDirection.Left;
                if (pRight.matcher(directionString).matches())
                    d = GoogleMapsDirection.TurnDirection.Right;
                if (uTurn.matcher(directionString).matches())
                    d = GoogleMapsDirection.TurnDirection.TurnAround;

                if (d == null)
                    d = GoogleMapsDirection.TurnDirection.Straight;

                GoogleMapsDirection payload = new GoogleMapsDirection.Builder()
                        .distance(distance.getText().toString().replaceAll("\\s+",""))
                        .eta(eta.getText().toString().replaceAll("\\s+",""))
                        .direction(direction.getText().toString().replaceAll("\\s+",""))
                        .tdirection(d)
                        .build();


                cr.setValue(payload.encode());
                mBtGatt.writeCharacteristic(cr);
            }
        }
    }
}
