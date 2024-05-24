
package org.artsoft.rocksndiamonds;

import org.libsdl.app.SDLActivity;

import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;
import android.util.Log;

public class rocksndiamonds extends SDLActivity {
    private static final String TAG = "RND";
    private String[] args;

    @Override
    protected String[] getArguments() {
        return args;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");

        // init program arguments
        args = new String[0];

        // prevent SDL from sending "drop file" event on app start; use program arguments instead
        Intent intent = getIntent();
        handleIntent(intent, true);
        intent.setData(null);

        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        Log.d(TAG, "onNewIntent");

        // handle file opened with "open with" when app is already running
        handleIntent(intent, false);
    }

    private void handleIntent(Intent intent, boolean onCreate) {
        Log.d(TAG, "handleIntent");

        Uri uri = intent.getData();
        if (uri == null) {
            Log.d(TAG, "handleIntent: uri == null");
            return;
        }

        if (onCreate) {
            Log.d(TAG, "handleIntent: starting app with file as argument");

            // app not running yet -- starting app with "--drop-file" argument
            setProgramArgs(uri);
        } else {
            Log.d(TAG, "handleIntent: sending drop event to running app");

            // app already running -- sending file as a "drop file" event
            sendUriAsDroppedFile(uri);
        }
    }

    public void sendUriAsDroppedFile(Uri uri) {
        SDLActivity.onNativeDropFile(getFileDescriptorStringFromUri(uri));
    }

    private int getFileDescriptorFromUri(Uri uri) {
        int fd = -1;

        try {
            ParcelFileDescriptor pfd = getContentResolver().openFileDescriptor(uri, "r");
            if (pfd == null) {
                throw new RuntimeException("pfd is null");
            }

            fd = pfd.dup().detachFd();
            pfd.close();
        } catch (Exception e) {
            Log.e(TAG, "Failed to convert URI " + uri.toString() + " to file descriptor", e);
        }

        return fd;
    }

    private String getFileDescriptorStringFromUri(Uri uri) {
        return "fd:" + getFileDescriptorFromUri(uri);
    }

    private void setProgramArgs(Uri uri) {
        Log.d(TAG, "setProgramArgs");

        // log some file details
        Cursor returnCursor = getContentResolver().query(uri, null, null, null, null);
        int nameIndex = returnCursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
        int sizeIndex = returnCursor.getColumnIndex(OpenableColumns.SIZE);
        returnCursor.moveToFirst();
        Log.e(TAG, "setProgramArgs: file name: " + returnCursor.getString(nameIndex));
        Log.e(TAG, "setProgramArgs: file size: " + Long.toString(returnCursor.getLong(sizeIndex)));

        String scheme = uri.getScheme();
        if (scheme != null) {
            if (scheme.equals("content")) {
                // convert URI to file descriptor
                String fd = getFileDescriptorStringFromUri(uri);
                Log.e(TAG, "setProgramArgs: setting argument to file descriptor: " + fd);
                args = new String[]{ "--drop-file", fd };
            } else if (scheme.equals("file")) {
                // directly use regular file
                String path = uri.getPath();
                Log.e(TAG, "setProgramArgs: setting argument to file path: " + path);
                args = new String[]{ "--drop-file", path };
            }
        }
    }
}
