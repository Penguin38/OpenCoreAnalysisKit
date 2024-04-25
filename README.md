# Getting Started

Install cmake 3.21.1+ on ubuntu 22.04, download cmake 3.21.1+, See https://cmake.org/download/

```
sudo apt-get install cmake
```
Install clang-12

```
sudo apt-get install clang-12
```
Dowload NDK r22+, See https://github.com/android/ndk/wiki/Unsupported-Downloads

```
export ANDROID_NDK=<path-to>
./build.sh $ANDROID_NDK
```

# Usage
Emulator version:
```
$ adb root
$ adb push <path-to>/output/aosp/debug/emulator/bin/core-parser /data/
$ adb shell
```

```
emu64xa:/ # chmod +x /data/core-parser
emu64xa:/ # /data/core-parser -c /sdcard/Android/data/penguin.opencore.tester/files/core.opencore.tester_6422_Thread-2_6550_1709932681
Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file ercept in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
See the License for the specific language governing permissions and
limitations under the License.

For bug reporting instructions, please see:
     https://github.com/Penguin38/OpenCoreAnalysisKit
Core load (0x787ec5231148) /sdcard/Android/data/penguin.opencore.tester/files/core.opencore.tester_6422_Thread-2_6550_1709932681
Core env: /sdcard/Android/data/penguin.opencore.tester/files/core.opencore.tester_6422_Thread-2_6550_1709932681
  * Machine: x86_64
  * PointSize: 64
  * PointMask: 0xffffffffffffffff
  * VabitsMask: 0xffffffffffffffff
  * Thread: 6422
Switch android(34) env.
Android env:
  * ID: UE1A.230829.036.A1
  * Name: sdk_gphone64_x86_64
  * Model: sdk_gphone64_x86_64
  * Manufacturer: Google
  * Brand: google
  * Hardware: ranchu
  * ABIS: x86_64,arm64-v8a
  * Incremental: 11228894
  * Release: 14
  * Security: 2023-09-05
  * Type: userdebug
  * Tag: dev-keys
  * Fingerprint: google/sdk_gphone64_x86_64/emu64xa:14/UE1A.230829.036.A1/11228894:userdebug/dev-keys
  * Time: 1702767675
  * Debuggable: <unknown>
  * Sdk: 34
core-parser>
```

```
core-parser> help
        core          exec       sysroot          mmap          auxv
        file           map          read         write      register
      thread       getprop         print         hprof        search
       class           top         space           dex           env
       shell          help        remote       version          quit

```

```
core-parser> space
TYPE   REGION                  ADDRESS             NAME
  5  [0x12c00000, 0x2ac00000)  0x75db0d608820  main space (region space)
  0  [0x70209000, 0x7033d840)  0x75dbad608430  /system/framework/x86_64/boot.art
  0  [0x704c2000, 0x704dfc88)  0x75dbad6086f0  /system/framework/x86_64/boot-core-libart.art
  0  [0x7050b000, 0x7051c7f8)  0x75dbad60c370  /system/framework/x86_64/boot-okhttp.art
  0  [0x70535000, 0x70553e58)  0x75dbad60a690  /system/framework/x86_64/boot-bouncycastle.art
  0  [0x70573000, 0x705738a8)  0x75dbad608f30  /system/framework/x86_64/boot-apache-xml.art
  0  [0x70574000, 0x70b01618)  0x75dbad60bf50  /system/framework/x86_64/boot-framework.art
  0  [0x71244000, 0x71244310)  0x75dbad608c70  /system/framework/x86_64/boot-framework-graphics.art
  0  [0x71245000, 0x71253bd8)  0x75dbad60ac10  /system/framework/x86_64/boot-ext.art
  0  [0x71264000, 0x712ea210)  0x75dbad60a950  /system/framework/x86_64/boot-telephony-common.art
  0  [0x713b1000, 0x713f4ee8)  0x75dbad60aed0  /system/framework/x86_64/boot-voip-common.art
  0  [0x71451000, 0x7146d790)  0x75dbad609610  /system/framework/x86_64/boot-ims-common.art
  0  [0x7149a000, 0x7154a300)  0x75dbad609e50  /system/framework/x86_64/boot-core-icu4j.art
  0  [0x725ee000, 0x728ca4a8)  0x75dbad60a7f0  /system/framework/x86_64/boot-framework-adservices.art
  2  [0x72c6b000, 0x7328a000)  0x75db8d621f90  Zygote space
  1  [0x7328a000, 0x73292000)  0x75dacd61c990  non moving space
  4  [0x40006000, 0x56e5d000)  0x75db2d609650  mem map large object space
```

```
core-parser> help top
Usage: top <NUM> [--alloc|-a] [--shallow|-s] [--native|-n] [--display|-d] [--app|--zygote|--image]
core-parser> top 20 -d --app -n
Address       Allocations       ShallowSize       NativeSize     ClassName
TOTAL               12772        189066695            116563
------------------------------------------------------------
0x70579788              2              100             52682     android.graphics.Bitmap
0x70584650             37              629             37000     android.os.BinderProxy
0x70577540             36             1296              3564     android.graphics.Typeface
0x7067fc68             31              992              3069     android.graphics.RenderNode
0x7067e2f8             21             2289              2079     android.graphics.Paint
0x707f8330             16             2240              1584     android.text.TextPaint
0x705be4a8              3               96              1536     android.view.SurfaceControl$Transaction
0x70584488              3               96              1500     android.os.Binder
0x70674c68              9              288               891     android.content.res.ResourcesImpl$ThemeImpl
0x705980a0              9              144               891     android.graphics.Matrix
0x7059f6d8              8              416               792     android.graphics.RecordingCanvas
0x70598410              6               96               594     android.graphics.Path
0x708f3248              1               36               500     android.window.WindowOnBackInvokedDispatcher$OnBackInvokedCallbackWrapper
0x708cd7b8              1               81               500     android.view.inputmethod.RemoteInputConnectionImpl
0x708cd5d0              1               36               500     android.view.inputmethod.RemoteInputConnectionImpl$1
0x708cd3d8              1               36               500     android.view.inputmethod.InputMethodManager$2
0x708cbfc8              1               36               500     android.view.accessibility.AccessibilityManager$1
0x708cbe20              1               32               500     android.view.WindowManagerGlobal$1
0x708cbc00              1               40               500     android.view.ViewRootImpl$W
0x70823c90              1               40               500     android.os.storage.StorageManager$ObbActionListener
```

```
core-parser> help hprof
Usage: hprof <FILE> [option]
       option:
              --visible|-v
              --quick|-q
core-parser> hprof /data/test.hprof
hprof: heap dump /data/test.hprof starting...
hprof: heap dump completed, scan objects (526126).
hprof: saved [/data/test.hprof].
```

```
core-parser> help thread
Usage: thread [tid] [options]
Options:
  --native|-n: show local threads.
  --java|-j: show java threads.
  --all|-a: show all thread.
core-parser> thread -a
 Id   Tid    Status                          Name
*1    6422   Native                          "main"
 6    6431   WaitingInMainSignalCatcherLoop  "Signal Catcher"
 7    6432   Native                          "perfetto_hprof_listener"
 8    6434   Native                          "Jit thread pool worker thread 0"
 9    6436   Waiting                         "ReferenceQueueDaemon"
 10   6438   Sleeping                        "FinalizerWatchdogDaemon"
 11   6437   Waiting                         "FinalizerDaemon"
 12   6435   WaitingPerformingGc             "HeapTaskDaemon"
 13   6440   Native                          "binder:6422_1"
 14   6441   Native                          "binder:6422_2"
 15   6433   WaitingInMainDebuggerLoop       "ADB-JDWP Connection Control Thread"
 16   6442   Native                          "binder:6422_3"
 17   6445   Native                          "Profile Saver"
 18   6446   Native                          "opencore-bg"
 19   6448   Native                          "RenderThread"
 20   6457   Native                          "SurfaceSyncGroupTimer"
 21   6459   Native                          "hwuiTask0"
 22   6460   Native                          "hwuiTask1"
 23   6461   Native                          "binder:6422_4"
 24   6464   Native                          "binder:6422_5"
 2    6550   Waiting                         "Thread-2"
 ---  6462   NotAttachJVM
```

```
core-parser> help p
Usage: print|p object -[br]
core-parser> p 0x12c00000
Size: 0x28
Padding: 0x1
Object Name: android.os.MessageQueue
    [0x24] private boolean mBlocked = true
    [0x08] private android.util.SparseArray mFileDescriptorRecords = 0x0
    [0x0c] private final java.util.ArrayList mIdleHandlers = 0x12e62918
    [0x10] android.os.Message mMessages = 0x0
    [0x20] private int mNextBarrierToken = 0x18
    [0x14] private android.os.MessageQueue$IdleHandler[] mPendingIdleHandlers = 0x12e62930
    [0x18] private long mPtr = 0x75daed63b7d0
    [0x25] private final boolean mQuitAllowed = false
    [0x26] private boolean mQuitting = false
  extends java.lang.Object
    [0x00] private transient java.lang.Class shadow$_klass_ = 0x70780340
    [0x04] private transient int shadow$_monitor_ = 0x10000000
```

```
core-parser> help search
Usage: search [CLASSNAME] [-r|--regex] [-i|--instanceof] [-o|--object] [-c|--class] [-s|--show]
core-parser> search MainActivity -r
[1] 0x12c00cb0 penguin.opencore.tester.MainActivity$4
[2] 0x12c00db0 penguin.opencore.tester.MainActivity$4
[3] 0x12c0aba8 penguin.opencore.tester.MainActivity
[4] 0x12d3cbc0 penguin.opencore.tester.MainActivity$1
[5] 0x12d3cca8 penguin.opencore.tester.MainActivity$2
[6] 0x12d3cd90 penguin.opencore.tester.MainActivity$3
[7] 0x12d3ce78 penguin.opencore.tester.MainActivity
```

```
core-parser> help class
Usage: class [CLASSNAME]
core-parser> class penguin.opencore.tester.MainActivity
[0x12d3ce78]
public class penguin.opencore.tester.MainActivity extends androidx.appcompat.app.AppCompatActivity
    implements android.view.LayoutInflater$Factory, android.view.LayoutInflater$Factory2, android.view.Window$Callback, android.view.KeyEvent$Callback, android.view.View$OnCreateContextMenuListener, android.content.ComponentCallbacks, android.content.ComponentCallbacks2, android.view.Window$OnWindowDismissedCallback, android.view.contentcapture.ContentCaptureManager$ContentCaptureClient, androidx.lifecycle.LifecycleOwner, androidx.core.view.KeyEventDispatcher$Component, androidx.lifecycle.ViewModelStoreOwner, androidx.savedstate.SavedStateRegistryOwner, androidx.activity.OnBackPressedDispatcherOwner, androidx.core.app.ActivityCompat$OnRequestPermissionsResultCallback, androidx.core.app.ActivityCompat$RequestPermissionsRequestCodeValidator, androidx.appcompat.app.AppCompatCallback, androidx.core.app.TaskStackBuilder$SupportParentable, androidx.appcompat.app.ActionBarDrawerToggle$DelegateProvider, android.view.View$OnClickListener, penguin.opencore.sdk.Coredump$Listener {
  Object instance fields:
  extends androidx.appcompat.app.AppCompatActivity
    [0x014c] private androidx.appcompat.app.AppCompatDelegate mDelegate
    [0x0150] private android.content.res.Resources mResources
  extends androidx.fragment.app.FragmentActivity
    [0x0144] boolean mCreated
    [0x0134] final androidx.lifecycle.LifecycleRegistry mFragmentLifecycleRegistry
    [0x0138] final androidx.fragment.app.FragmentController mFragments
    [0x0140] int mNextCandidateRequestIndex
    [0x013c] androidx.collection.SparseArrayCompat mPendingFragmentActivityResults
    [0x0145] boolean mRequestedPermissionsFromFragment
    [0x0146] boolean mResumed
    [0x0147] boolean mStartedActivityFromFragment
    [0x0148] boolean mStartedIntentSenderFromFragment
    [0x0149] boolean mStopped
  extends androidx.activity.ComponentActivity
    [0x0130] private int mContentLayoutId
    [0x0120] private final androidx.lifecycle.LifecycleRegistry mLifecycleRegistry
    [0x0124] private final androidx.activity.OnBackPressedDispatcher mOnBackPressedDispatcher
    [0x0128] private final androidx.savedstate.SavedStateRegistryController mSavedStateRegistryController
    [0x012c] private androidx.lifecycle.ViewModelStore mViewModelStore
  extends androidx.core.app.ComponentActivity
    [0x0118] private androidx.collection.SimpleArrayMap mExtraDataMap
    [0x011c] private androidx.lifecycle.LifecycleRegistry mLifecycleRegistry
  extends android.app.Activity
    [0x0028] android.app.ActionBar mActionBar
    [0x00e8] private int mActionModeTypeStarting
    [0x002c] android.content.pm.ActivityInfo mActivityInfo
    [0x0030] private final java.util.ArrayList mActivityLifecycleCallbacks
    [0x0034] android.app.ActivityTransitionState mActivityTransitionState
    [0x0038] private android.app.Application mApplication
    [0x003c] private android.os.IBinder mAssistToken
    [0x0040] private android.view.autofill.AutofillClientController mAutofillClientController
    [0x0044] private android.content.ComponentCallbacksController mCallbacksController
    [0x0100] boolean mCalled
    [0x0101] private boolean mCanEnterPictureInPicture
    [0x0102] private boolean mChangeCanvasToTranslucent
    [0x0103] boolean mChangingConfigurations
    [0x0048] private android.content.ComponentName mComponent
    [0x00ec] int mConfigChangeFlags
    [0x004c] private android.view.contentcapture.ContentCaptureManager mContentCaptureManager
    [0x0050] android.content.res.Configuration mCurrentConfig
    [0x0054] android.view.View mDecor
    [0x0058] private android.window.OnBackInvokedCallback mDefaultBackCallback
    [0x00f0] private int mDefaultKeyMode
    [0x005c] private android.text.SpannableStringBuilder mDefaultKeySsb
    [0x0104] private boolean mDestroyed
    [0x0105] private boolean mDoReportFullyDrawn
    [0x0060] private com.android.internal.util.dump.DumpableContainerImpl mDumpableContainer
    [0x0064] java.lang.String mEmbeddedID
    [0x0106] private boolean mEnableDefaultActionBarUp
    [0x0107] boolean mEnterAnimationComplete
    [0x0068] android.app.SharedElementCallback mEnterTransitionListener
    [0x006c] android.app.SharedElementCallback mExitTransitionListener
    [0x0108] boolean mFinished
    [0x0070] final android.app.FragmentController mFragments
    [0x0074] final android.os.Handler mHandler
    [0x0109] private boolean mHasCurrentPermissionsRequest
    [0x00f4] private int mIdent
    [0x0078] private final java.lang.Object mInstanceTracker
    [0x007c] private android.app.Instrumentation mInstrumentation
    [0x0080] android.content.Intent mIntent
    [0x010a] private boolean mIsInMultiWindowMode
    [0x010b] boolean mIsInPictureInPictureMode
    [0x0084] android.app.Activity$NonConfigurationInstances mLastNonConfigurationInstances
    [0x010c] boolean mLaunchedFromBubble
    [0x0088] android.app.ActivityThread mMainThread
    [0x008c] private final java.util.ArrayList mManagedCursors
    [0x0090] private android.util.SparseArray mManagedDialogs
    [0x0094] private android.view.MenuInflater mMenuInflater
    [0x0098] android.app.Activity mParent
    [0x009c] android.app.ActivityOptions mPendingOptions
    [0x00a0] java.lang.String mReferrer
    [0x010d] private boolean mRestoredFromBundle
    [0x00f8] int mResultCode
    [0x00a4] android.content.Intent mResultData
    [0x010e] boolean mResumed
    [0x00a8] private android.app.ScreenCaptureCallbackHandler mScreenCaptureCallbackHandler
    [0x00ac] private android.view.SearchEvent mSearchEvent
    [0x00b0] private android.app.SearchManager mSearchManager
    [0x00b4] private android.os.IBinder mShareableActivityToken
    [0x010f] private boolean mShouldDockBigOverlays
    [0x00b8] private android.window.SplashScreen mSplashScreen
    [0x0110] boolean mStartedActivity
    [0x0111] boolean mStopped
    [0x00bc] private android.app.ActivityManager$TaskDescription mTaskDescription
    [0x00c0] private java.lang.CharSequence mTitle
    [0x00fc] private int mTitleColor
    [0x0112] private boolean mTitleReady
    [0x00c4] private android.os.IBinder mToken
    [0x00c8] private android.app.Activity$TranslucentConversionListener mTranslucentCallback
    [0x00cc] private java.lang.Thread mUiThread
    [0x00d0] private android.view.translation.UiTranslationController mUiTranslationController
    [0x0113] boolean mVisibleFromClient
    [0x0114] boolean mVisibleFromServer
    [0x00d4] private com.android.internal.app.IVoiceInteractionManagerService mVoiceInteractionManagerService
    [0x00d8] android.app.VoiceInteractor mVoiceInteractor
    [0x00dc] private android.view.Window mWindow
    [0x0115] boolean mWindowAdded
    [0x00e0] private final android.view.Window$WindowControllerCallback mWindowControllerCallback
    [0x00e4] private android.view.WindowManager mWindowManager
  extends android.view.ContextThemeWrapper
    [0x0014] private android.view.LayoutInflater mInflater
    [0x0018] private android.content.res.Configuration mOverrideConfiguration
    [0x001c] private android.content.res.Resources mResources
    [0x0020] private android.content.res.Resources$Theme mTheme
    [0x0024] private int mThemeResource
  extends android.content.ContextWrapper
    [0x0008] android.content.Context mBase
    [0x000c] public java.util.List mCallbacksRegisteredToSuper
    [0x0010] private final java.lang.Object mLock
  extends android.content.Context
  extends java.lang.Object
    [0x0000] private transient java.lang.Class shadow$_klass_
    [0x0004] private transient int shadow$_monitor_
}
```
