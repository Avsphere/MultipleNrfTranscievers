package com.example.hhaysmit.agilitypitest;

import android.os.ParcelUuid;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;


public class MainActivity extends AppCompatActivity {

    private Button ledOn;
    private Button ledOff;

    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private OutputStream outStream = null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.i("startup", "creating");
        ledOn = (Button) findViewById(R.id.led_on);
        ledOff = (Button) findViewById(R.id.led_off);

        btAdapter = BluetoothAdapter.getDefaultAdapter();
        Log.i("adapter", "" + btAdapter.equals(null));
        connectToDevice();
    }

    private void connectToDevice(){
        Log.i("connecting", "connecting to device");
        Set<BluetoothDevice> pairedDevices = btAdapter.getBondedDevices();
        if(pairedDevices.isEmpty()){
            Log.i("?", "no paired device");
            return;
        }
        if(pairedDevices.size() > 1){
            Log.i("h", "only one device can be paired");
            return;
        }
        if(pairedDevices.size() == 1){
        BluetoothDevice btDevice = pairedDevices.iterator().next();
        ParcelUuid[] parcelUuids = btDevice.getUuids();
            Log.i("uuid", "" + parcelUuids.length);
        UUID uuid = parcelUuids[0].getUuid();
            Log.i("uuid", "" + uuid);
        try {
            btSocket = btDevice.createRfcommSocketToServiceRecord(uuid);
            btAdapter.cancelDiscovery();
        } catch(IOException e) {
            Log.e("socket", "failed to create socket");
            return;
        }
            try {
                btSocket.connect();
            }catch(IOException err){
                Log.e("socket", "Failed to connect to socket");
                try{
                    btSocket.close();
                } catch(IOException e1){
                    Log.e("socket", "failed to close socket");
                }
                return;
            }
            if(!btSocket.isConnected()){
                Log.i("socket is connected", "failed to connect to socket");
                return;
            }
            try{
            outStream = btSocket.getOutputStream();
            } catch(IOException error){
                Log.i("outstream", "failed to create outstream");
                return;
            }

            Log.i("outstream", "" + outStream.equals(null));
        }
    }

    public void ledOn(View view){
        Log.i("on", "attempting to turn on");
        sendData("1");
    }

    public void ledOff(View view){
        Log.i("off", "attempting to turn off");
        sendData("0");
    }

    private void sendData(String message){
        byte[] msgBuffer = message.getBytes();
        try{
            outStream.write(msgBuffer);
        }catch(IOException e){
            Log.i("outstream", "Failed to write to outstream");
            return;
        }
    }
}
