package me.brosa.mapsup;

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
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.IBinder;
import android.service.notification.StatusBarNotification;
import android.util.Log;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.RemoteViews;
import android.widget.TextView;
import android.widget.Toast;

import java.io.InputStream;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

import static android.bluetooth.BluetoothGatt.GATT_SUCCESS;


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
    private int mLastTransmitionCode = 0;

    private Set<BluetoothDevice> mDevices = new HashSet<>();

    private Thread mBtScanThread = null;

    private volatile boolean mScanActive = false;
    private boolean mConnected = false;
    private boolean inFlight = false;

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
                //scanForDevices();
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == GATT_SUCCESS) {
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

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            inFlight = false;
            if (status != GATT_SUCCESS) {
                Log.e(DEBUG_TAG, "Failed to write characteristic ");
            } else {
                Log.d(DEBUG_TAG, "SUCCESS");
            }
            super.onCharacteristicWrite(gatt, characteristic, status);
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
        if (sbn.getPackageName().equals("com.google.android.apps.maps") && !sbn.isClearable()) {
            if (mBtDevice == null) {
                scanForDevices();
            }
            sendGoogleMapsDirections(sbn.getNotification());
        }

        super.onNotificationPosted(sbn);
    }


    private void sendGoogleMapsDirections(Notification n) {
        RemoteViews rv = n.bigContentView;
        RelativeLayout rl = (RelativeLayout) rv.apply(getApplicationContext(), null);
        ImageView turnArrow = (ImageView) rl.getChildAt(0);

        //Log.d(DEBUG_TAG, Integer.toHexString(turnArrow.hashCode()));
        //Log.d(DEBUG_TAG, Integer.toHexString(turnArrow.getDrawable().getConstantState().hashCode()));

        LinearLayout ll = (LinearLayout) ((LinearLayout) rl.getChildAt(1)).getChildAt(0);
        TextView distance = (TextView) ll.getChildAt(0);
        TextView direction = (TextView) ll.getChildAt(1);
        TextView eta = (TextView) ll.getChildAt(2);

        //Log.d(DEBUG_TAG, distance.getText().toString());
        //Log.d(DEBUG_TAG, direction.getText().toString());
        //Log.d(DEBUG_TAG, eta.getText().toString());

        if (mConnected && mBtGattService != null) {

            BluetoothGattCharacteristic cr = mBtGattService.getCharacteristic(NotificationListenerService.UUID_BLE_SHIELD_TX);
            GoogleMapsDirection.TurnDirection d = null;
            String directionString = direction.getText().toString();

            Pattern pStraight = Pattern.compile("continue|straight|ahead|stay", Pattern.CASE_INSENSITIVE);
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
                    .distance("Hello world this is a test of distance!!!!!!")
                    .tdirection(d)
                    .build();
            /*if (payload.hashCode() == mLastTransmitionCode)
                return;*/
            mLastTransmitionCode = payload.hashCode();
            byte payloadEncoding[] = payload.encode();
            byte payloadFrame[] = new byte[payloadEncoding.length + 4];
            payloadFrame[0] = new Integer(0xA5).byteValue();
            payloadFrame[1] = new Integer(payloadEncoding.length).byteValue();
            payloadFrame[2] = 0;

            int checksum = 0xA5 ^ payloadEncoding.length;
            for (int i = 0; i < payloadEncoding.length; i++) {
                payloadFrame[3 + i] = payloadEncoding[i];
                checksum ^= payloadEncoding[i];
            }
            payloadFrame[payloadFrame.length - 1] = new Integer(checksum).byteValue();

            int remainingBytes = payloadFrame.length;
            int bufferPtr = 0;
            for (int i = 0; i < (int)(payloadFrame.length / 20) + 1; i++) {
                byte currentFrame[] = new byte[Math.min(remainingBytes, 20)];
                for (int j = 0; j < currentFrame.length; j++) {
                    currentFrame[j] = payloadFrame[bufferPtr++];
                    remainingBytes--;
                }
                cr.setValue(currentFrame);
                try {
                    Thread.sleep(250);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if (!mBtGatt.writeCharacteristic(cr)) {
                    inFlight = true;
                    Log.e(DEBUG_TAG, "Failed to initiate write of characteristic");
                }
                Log.d(DEBUG_TAG, "Wrote frame of " + String.valueOf(currentFrame.length) + " bytes");
            }
        }
    }
}
