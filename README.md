# Getting Started

Install cmake 3.21.1+ on ubuntu 22.04, download cmake 3.21.1+, please see:
```
https://cmake.org/download/
```
```
sudo apt-get install cmake
```
Install clang-12

```
sudo apt-get install clang-12
```
Dowload NDK r22+, please see:
```
https://github.com/android/ndk/wiki/Unsupported-Downloads
```
```
export ANDROID_NDK=<path-to>
./build.sh $ANDROID_NDK
```
# Compatible

| sdk          | arm64 | arm  | x86_64 | x86  | riscv64 |
|:------------:|:-----:|:----:|:------:|:----:|:-------:|
|Android-8.0 (26) |   √   |  √   |   √    |   √  |    ?    |
|Android-8.1 (27) |   -   |  -   |   -    |   -  |    -    |
|Android-9.0 (28) |   √   |  √   |   √    |   √  |    ?    |
|Android-10.0(29) |   √   |  √   |   √    |   √  |    ?    |
|Android-11.0(30) |   √   |  √   |   √    |   √  |    ?    |
|Android-12.0(31) |   √   |  √   |   √    |   √  |    ?    |
|Android-12.1(32) |   √   |  √   |   √    |   √  |    ?    |
|Android-13.0(33) |   √   |  √   |   √    |   √  |    ?    |
|Android-14.0(34) |   √   |  √   |   √    |   √  |    ?    |
|Android-15.0(35) |   √   |  -   |   √    |   -  |    ?    |

# Usage
Emulator version:
```
$ adb root
$ adb push <path-to>/output/aosp/debug/emulator/bin/core-parser /data/
$ adb shell
```

```
Usage: core-parser [Option..]
Option:
    --core|-c <COREFILE>
    --pid|-p <PID>
    --machine|-m <ARCH>{ arm64, arm, x86_64, x86, riscv64 }
    --sdk <SDK>{ 26 ~ 35 }
Exp:
  core-parser -c /tmp/tmp.core
  core-parser -p 1 -m arm64
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
      thread     backtrace         frame       getprop         print  
       hprof        search         class           top         space  
         dex        method           env         shell        plugin  
        help        remote          fake          time       version  
        quit
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
core-parser> class java.lang.Object
[0x7024aa78]
public class java.lang.Object {
  // Object instance fields:
    [0x00] private transient java.lang.Class shadow$_klass_
    [0x04] private transient int shadow$_monitor_

  // Methods:
    [0x70416968] public void java.lang.Object.<init>()
    [0x70416988] static int java.lang.Object.identityHashCode(java.lang.Object)
    [0x704169a8] private static native int java.lang.Object.identityHashCodeNative(java.lang.Object)
    [0x704169c8] private native java.lang.Object java.lang.Object.internalClone()
    [0x704169e8] protected java.lang.Object java.lang.Object.clone()
    [0x70416a08] public boolean java.lang.Object.equals(java.lang.Object)
    [0x70416a28] protected void java.lang.Object.finalize()
    [0x70416a48] public final java.lang.Class java.lang.Object.getClass()
    [0x70416a68] public int java.lang.Object.hashCode()
    [0x70416a88] public final native void java.lang.Object.notify()
    [0x70416aa8] public final native void java.lang.Object.notifyAll()
    [0x70416ac8] public java.lang.String java.lang.Object.toString()
    [0x70416ae8] public final void java.lang.Object.wait()
    [0x70416b08] public final void java.lang.Object.wait(long)
    [0x70416b28] public final native void java.lang.Object.wait(long, int)
}
```

```
core-parser> help method
Usage: method <ArtMethod> [option..]
Option:
    --dex-dump: show dalvik byte codes
    --oat-dump: show oat machine codes
    --binary|-b: show ArtMethod memory
    --inst|-i: show instpc byte code
    --num|-n: maxline num
core-parser> method 0x70416a48 --dex-dump -b
public final java.lang.Class java.lang.Object.getClass() [dex_method_idx=3089]
DEX CODE:
  0x75d9a86fde44: 1054 0628                | iget-object v0, v1, Ljava/lang/Object;.shadow$_klass_:Ljava/lang/Class; // field@1576
  0x75d9a86fde48: 0011                     | return-object v0
Binary:
70416a48: 503800117024aa78  0000000300000c11  x.$p..8P........
70416a58: 000075d9a86fde34  000075d9a9591b50  4.o..u..P.Y..u..

```

```
core-parser> dex
DEXCACHE    REGION                   FLAGS NAME
0x12d3f050  [75dc65711000, 75dc65714000)  r--  /data/app/~~oKR0P01OBRNtuKZwiy0k7A==/penguin.opencore.tester-Oa4B9U4RQb20If694Ti0qQ==/base.apk!classes3.dex [*]
0x12d3c728  [75dc52779000, 75dc527ac000)  r--  /data/app/~~oKR0P01OBRNtuKZwiy0k7A==/penguin.opencore.tester-Oa4B9U4RQb20If694Ti0qQ==/base.apk!classes2.dex [*]
0x725ee800  [75dc654d0000, 75dc654d2000)  r--  /apex/com.android.sdkext/javalib/framework-sdkextensions.jar [*]
0x12c217d8  [75d942208000, 75d942400000)  r--  /data/app/~~oKR0P01OBRNtuKZwiy0k7A==/penguin.opencore.tester-Oa4B9U4RQb20If694Ti0qQ==/base.apk [*]
0x725ee790  [75dc65495000, 75dc65499000)  r--  /apex/com.android.scheduling/javalib/framework-scheduling.jar [*]
0x725ee720  [75dc64e52000, 75dc64e87000)  r--  /apex/com.android.permission/javalib/framework-permission-s.jar [*]
0x725ee6b0  [75dc658bb000, 75dc658bc000)  r--  /apex/com.android.permission/javalib/framework-permission.jar [*]
0x725ee640  [75dc651f0000, 75dc65208000)  r--  /apex/com.android.os.statsd/javalib/framework-statsd.jar [*]
0x725ee870  [75d9a779d000, 75d9a7931000)  r--  /apex/com.android.tethering/javalib/framework-connectivity.jar [*]
```

```
core-parser> map
LINKMAP       REGION                   FLAGS NAME
0x75dc659080e0  [60969cb26000, 60969cb28000)  r--  /system/bin/app_process64 [*]
0x75dc65b9e210  [75dc65a44000, 75dc65a8b000)  r--  /system/bin/linker64 [*]
0x75dc65908338  [7ffe0b188000, 7ffe0b189000)  r-x  [vdso] [*]
0x75dc65908590  [75dc4d4f8000, 75dc4d5d5000)  r--  /system/lib64/libandroid_runtime.so [*]
0x75dc659087e8  [75dc522d6000, 75dc52338000)  r--  /system/lib64/libbinder.so [*]
```

```
core-parser> help exec
Usage: exec /system/bin/app_process64
core-parser> exec /system/bin/app_process64
Mmap segment [60969cb26000, 60969cb28000) /system/bin/app_process64 [0]
Mmap segment [60969cb28000, 60969cb2a000) /system/bin/app_process64 [1000]
```

```
core-parser> help sysroot
Usage: sysroot /system:/apex:/vendor --[map|dex]
core-parser> sysroot /system --map
Mmap segment [60969cb26000, 60969cb28000) /system/bin/app_process64 [0]
Mmap segment [60969cb28000, 60969cb2a000) /system/bin/app_process64 [1000]
Mmap segment [75dc65a44000, 75dc65a8b000) /system/bin/linker64 [0]
Mmap segment [75dc65a8b000, 75dc65b8e000) /system/bin/linker64 [46000]
Mmap segment [75dc4d4f8000, 75dc4d5d5000) /system/lib64/libandroid_runtime.so [0]
...
core-parser> sysroot /system --dex
Mmap segment [75dc65357000, 75dc6539f000) /system/framework/ext.jar [0]
Mmap segment [75d9a7c00000, 75d9a7fb8000) /system/framework/telephony-common.jar [0]
Mmap segment [75dc658c1000, 75dc658c2000) /system/framework/framework-graphics.jar [0]
Mmap segment [75d9a8f9a000, 75d9a90e8000) /system/framework/framework.jar [24d8000]
Mmap segment [75d9a856d000, 75d9a8600000) /system/framework/ims-common.jar [0]
Mmap segment [75d9a3fa8000, 75d9a4975000) /system/framework/framework.jar [11a8000]
...
```

```
core-parser> help wd
Usage: write|wd <Address> <Option>
Option:
    --string|-s <STRING>:
    --value|-v <VALUE>:
core-parser> wd 75d9a3fa8000 -v 0x0
New overlay [75d9a3fa8000, 75d9a4975000)
```

```
core-parser> help rd
Usage: read|rd begin [-e end] [--opt] [-f path]
         opt: --origin --mmap --overlay
core-parser> rd 75d9a3fa8000 -e 75d9a3fa8020
75d9a3fa8000: 0000000000000000  0202020202020202  ................
75d9a3fa8010: 0202020202020202  0230020202020202  ..............0.

core-parser> rd 75d9a3fa8000 -e 75d9a3fa8020 --origin
75d9a3fa8000: 0202020202020202  0202020202020202  ................
75d9a3fa8010: 0202020202020202  0230020202020202  ..............0.
```

```
core-parser> auxv
    21   AT_SYSINFO_EHDR  0x7ffe0b188000
    33    AT_MINSIGSTKSZ  0x5a0
    10          AT_HWCAP  0x178afbfd
     6         AT_PAGESZ  0x1000
    11         AT_CLKTCK  0x64
     3           AT_PHDR  0x60969cb26040
     4          AT_PHENT  0x38
     5          AT_PHNUM  0xa
     7           AT_BASE  0x75dc65a44000
     8          AT_FLAGS  0x0
     9          AT_ENTRY  0x60969cb28900
     b            AT_UID  0x0
     c           AT_EUID  0x0
     d            AT_GID  0x0
     e           AT_EGID  0x0
    17         AT_SECURE  0x1
    19         AT_RANDOM  0x7ffe0b052999
    1a         AT_HWCAP2  0x0
    1f         AT_EXECFN  0x7ffe0b054fde /system/bin/app_process64
     f       AT_PLATFORM  0x7ffe0b0529a9 x86_64
     0           AT_NULL  0x0
```

```
core-parser> file
[12c00000, 2ac00000)  00000000  [anon:dalvik-main space (region space)]
[40006000, 40017000)  00000000  [anon:dalvik-large object space allocation]
[40018000, 40029000)  00000000  [anon:dalvik-large object space allocation]
[40033000, 40044000)  00000000  [anon:dalvik-large object space allocation]
[4004f000, 40093000)  00000000  [anon:dalvik-large object space allocation]
...
```

```
core-parser> thread 3962
Current thread is 3962

core-parser> bt
"HeapTaskDaemon" tid=9 WaitingForTaskProcessor
  | group="system" daemon=1 prio=5 target=0x701b1058
  | sysTid=3962 sCount=0 flags=0 obj=0x12c11530 self=0x70ef11c6b140
  | stack=0x70ed38d4c000-0x70ed38d4e000 stackSize=0x102cf0 handle=0x70ed38e4ecf0
  | held mutexes=
  rax 0xfffffffffffffdfc  rbx 0x000070ee11c58da0  rcx 0x000070f053bd3dc8  rdx 0x000000000000000a
  r8  0x0000000000000000  r9  0xb4e7000000000000  r10 0x000070ed38e4e770  r11 0x0000000000000246
  r12 0x000070ed38e4e770  r13 0x000000000000000a  r14 0x000070ef11c6b140  r15 0x0000000000000000
  rdi 0x000070ee11c58db0  rsi 0x0000000000000080
  rbp 0x000070ee11c58db0  rsp 0x000070ed38e4e748  rip 0x000070f053bd3dc8  flags 0x0000000000000246
  ds 0x00000000  es 0x00000000  fs 0x00000000  gs 0x00000000  cs 0x00000033  ss 0x0000002b
  JavaKt: #0  0000000000000000  dalvik.system.VMRuntime.runHeapTasks()
  JavaKt: #1  000070ed9f07bea6  java.lang.Daemons$HeapTaskDaemon.runInternal()
  JavaKt: #2  000070ed9f07b4f2  java.lang.Daemons$Daemon.run()
  JavaKt: #3  000070ed9f1d1740  java.lang.Thread.run()

core-parser> f 3
  JavaKt: #3  000070ed9f1d1740  java.lang.Thread.run()
  {
      art::ArtMethod: 0x7011b230
      shadow_frame: 0x0
      quick_frame: 0x70ed38e4e9f0
      dex_pc_ptr: 0x70ed9f1d1740
      frame_pc: 0x70ed9f96a7e6
      method_header: 0x70ed9f96038c

      DEX CODE:
      0x70ed9f1d1738: 1054 05fd                | iget-object v0, v1, Ljava/lang/Thread;.target:Ljava/lang/Runnable; // field@1533
      0x70ed9f1d173c: 0038 0005                | if-eqz v0, 0x70ed9f1d1746 //+5
      0x70ed9f1d1740: 1072 0c2d 0000           | invoke-interface {v0}, void java.lang.Runnable.run() // method@3117
      {
          v0 = 0x701b1058    v1 = 0x12c11530
      }

      OAT CODE:
      {
          rbx = 0x000070ed38e4ea98    rbp = 0x000070ed9f1d1740    r12 = 0x000070ed38e4ea18    r13 = 0x000070ed9f960a80
          r14 = 0x000070ed38e4ea10    r15 = 0x000070ed9f96a7e6
      }
  }
```

```
core-parser> help remote
Usage: remote <COMMAND> [option] ...
Command:
    core  hook  rd  wd
    pause

Usage: remote core -p <PID> -m <MACHINE> [Option]...
Option:
   --pid|-p <PID>
   --dir|-d <DIR>
   --machine|-m <Machine>
Machine:
     { arm64, arm, x86_64, x86, riscv64 }
   --output|-o <COREFILE>
   --filter|-f <Filter>
Filter:
     0x01: filter-special-vma
     0x02: filter-file-vma
     0x04: filter-shared-vma
     0x08: filter-sanitizer-shadow-vma
     0x10: filter-non-read-vma

remote rd -p <PID> <BEGIN> -e <END>
remote wd -p <PID> <ADDRESS> [-s|-v] <VALUE>
remote pause <PID ...> [-a]

core-parser> remote core -p 1 -m x86_64 -d /data -f 0x18
Coredump /data/core.init_1_1718900269 ...
Finish done.
core-parser> core /data/core.init_1_1718900269
Core load (0x7256f0c21090) /data/core.init_1_1718900269
Core env: /data/core.init_1_1718900269
  * Machine: x86_64
  * Bits: 64
  * PointSize: 8
  * PointMask: 0xffffffffffffffff
  * VabitsMask: 0xffffffffffffffff
  * Thread: 1
  ...
```

```
core-parser> remote wd -p 1 7fb989794000 -s PenguinLetsGo
core-parser> remote rd -p 1 7fb989794000 -e 7fb989794030
7fb989794000: 4c6e6975676e6550  0000006f47737465  PenguinLetsGo...
7fb989794010: 00000001003e0003  0000000000068ab0  ..>.............
7fb989794020: 0000000000000040  0000000000198c20  @...............
```

```
core-parser> help fake
Usage: fake <COMMAND> [option] ...
Command:
    core  map

Usage: fake core <Option> ...
Option:
   --tomb|-t <TOMBSTONE>
   --restore|-r: rebuild current environment core.
   --output|-o <COREFILE>

Usage: fake map

core-parser> fake core -r -o fake.core
```
