# Dependencies
[Capstone Disassembly Framework](https://github.com/capstone-engine/capstone)

# Techincal System
![core-analysis](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/5e18eb85fd5047f2a017e4ffff075339~tplv-k3u1fbpfcp-jj-mark:0:0:0:0:q75.image#?w=3028&h=1480&s=318380&e=png&b=fbf9f9)

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
$ adb push <path-to>/output/aosp/debug/android/x86_64/bin/core-parser /data/
$ adb shell
```

```
Usage: core-parser [OPTION]
Option:
    -c, --core <COREFILE>    load core-parser from corefile
    -p, --pid <PID>          load core-parser from target process
    -m, --machine <ARCH>     arch support arm64, arm, x86_64, x86, riscv64
        --sdk <SDK>          sdk support 26 ~ 35
        --non-quick          load core-parser no filter non-read vma.
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
      thread     backtrace         frame   disassemble       getprop
       print         hprof        search         class           top
       space           dex        method        logcat       dumpsys
         env           cxx         shell        plugin          help
      remote          fake          time       version          quit
```

```
core-parser> help space
Usage: space [OPTION] [TYPE]
Option:
    -c, --check   check space bad object.
Type: {--app, --zygote, --image, --fake}

core-parser> space
TYPE   REGION                  ADDRESS             NAME
  5  [0x12c00000, 0x2ac00000)  0x75db0d608820  main space (region space)
  0  [0x70209000, 0x7033d840)  0x75dbad608430  /system/framework/x86_64/boot.art
  0  [0x704c2000, 0x704dfc88)  0x75dbad6086f0  /system/framework/x86_64/boot-core-libart.art
  ...

core-parser> space --check --app
ERROR: Region:[0x12c00000, 0x12c00018) main space (region space) has bad object!!
```

```
core-parser> help top
Usage: top <NUM> [OPTION] [TYPE]
Option:
    -a, --alloc     order by allocation
    -s, --shallow   order by shallow
    -n, --native    order by native
    -d, --display   show class name
Type: {--app, --zygote, --image, --fake}

core-parser> top 10 -d
Address       Allocations      ShallowSize        NativeSize     ClassName
TOTAL              136939          8045084            108415
------------------------------------------------------------
0x6f817d58          43562          2629504                 0     java.lang.String
0x6f7fdd30          14281          1405792                 0     long[]
0x6f7992c0          12084           479956                 0     java.lang.Object[]
0x6f824fd0           9405           225720                 0     java.util.HashMap$Node
0x6f7fda18           4689          1033816                 0     int[]
0x6f7fa7b0           3457           110624                 0     java.lang.ref.SoftReference
0x6f835118           3381            40572                 0     java.lang.Integer
0x6f8420e8           2684            85888                 0     java.util.LinkedHashMap$LinkedHashMapEntry
0x6fab25d0           2293            45860                 0     android.icu.util.CaseInsensitiveString
0x6f865b80           1859            37180                 0     java.util.ArrayList

core-parser> top 10 -d -s --app
Address       Allocations      ShallowSize        NativeSize     ClassName
TOTAL                7592          1943315            104175
------------------------------------------------------------
0x6f7fda18            322           561344                 0     int[]
0x6f817d58           1504           509408                 0     java.lang.String
0x6f799100              9           299177                 0     byte[]
0x6f7fd688             18           247138                 0     char[]
0x6f7fdd30            390           113576                 0     long[]
0x6f7992c0            577            27196                 0     java.lang.Object[]
0x6f865b80            335             6700                 0     java.util.ArrayList
0x6f79ba88            174             6264                 0     sun.misc.Cleaner
0x70101c18            258             6192                 0     android.graphics.Rect
0x70360328             40             5600                 0     android.animation.ObjectAnimator
```

```
core-parser> help hprof
Usage: hprof <FILE> [OPTION]
Option:
    -v, --visible     show hprof detail
    -q, --quick       fast dump hprof

core-parser> hprof /tmp/1.hprof
hprof: heap dump /tmp/1.hprof starting...
hprof: heap dump completed, scan objects (306330).
hprof: saved [/tmp/1.hprof].
```

```
core-parser> help thread
Usage: thread [TID] [OPTION]
Option:
    -n, --native    show local threads
    -j, --java      show jvm threads
    -a, --all       show all thread

core-parser> thread -a
 ID   TID    STATUS                          NAME
*1    6118   Runnable                        "main"
 2    6125   Native                          "Runtime worker thread 0"
 3    6128   Native                          "Runtime worker thread 3"
 4    6126   Native                          "Runtime worker thread 1"
 5    6129   WaitingInMainSignalCatcherLoop  "Signal Catcher"
 6    6127   Native                          "Runtime worker thread 2"
...

core-parser> thread -n
 ID     TARGET TID        FRAME
*1      Thread 6118       0x79185c88945f  /apex/com.android.art/lib64/libart.so
 2      Thread 6125       0x791aef6632a8  /apex/com.android.runtime/lib64/bionic/libc.so
 3      Thread 6126       0x791aef6632a8  /apex/com.android.runtime/lib64/bionic/libc.so
 4      Thread 6127       0x791aef6632a8  /apex/com.android.runtime/lib64/bionic/libc.so
...

core-parser> thread
Current thread is 6118

core-parser> thread 6133
Current thread is 6133
```

```
core-parser> help p
Usage: print|p <OBJECT> [OPTION..]
Option:
    -b, --binary       show object memory
    -r, --ref <DEEP>   show object's ref
    -f, --format       object format dump
    -x, --hex          basic type hex print

core-parser> p 0x12c00000
Size: 0x18
Object Name: java.lang.ref.WeakReference
  // extends java.lang.ref.Reference
    [0x14] volatile java.lang.Object referent = 0x12c00018
    [0x10] java.lang.ref.Reference queueNext = 0x0
    [0x0c] final java.lang.ref.ReferenceQueue queue = 0x0
    [0x08] java.lang.ref.Reference pendingNext = 0x0
  // extends java.lang.Object
    [0x04] private transient int shadow$_monitor_ = 0
    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f819828

core-parser> p 0x12c00000 -b -r 1 -x
Size: 0x18
Object Name: java.lang.ref.WeakReference
  // extends java.lang.ref.Reference
    [0x14] volatile java.lang.Object referent = 0x12c00018
    [0x10] java.lang.ref.Reference queueNext = 0x0
    [0x0c] final java.lang.ref.ReferenceQueue queue = 0x0
    [0x08] java.lang.ref.Reference pendingNext = 0x0
  // extends java.lang.Object
    [0x04] private transient int shadow$_monitor_ = 0x0
    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f819828
Reference:
  --> 0x7010eac0 com.android.internal.os.BinderInternal
Binary:
12c00000: 000000006f819828  0000000000000000  (..o............
12c00010: 12c0001800000000  00000000704662b0  .........bFp....
```

```
core-parser> help search
Usage: search <CLASSNAME> [OPTION..] [TYPE]
Option:
    -r, --regex        regular expression search
    -i, --instanceof   search by instance of class
    -o, --object       only search object
    -c, --class        only search class
    -p, --print        object print detail
    -x, --hex          basic type hex print
Type: {--app, --zygote, --image, --fake}

core-parser> search android.app.Activity -i -o --app --print
[1] 0x13050cc8 penguin.opencore.tester.MainActivity
Size: 0x130
Object Name: penguin.opencore.tester.MainActivity
  // extends androidx.appcompat.app.AppCompatActivity
    [0x12c] private android.content.res.Resources mResources = 0x0
    [0x128] private androidx.appcompat.app.AppCompatDelegate mDelegate = 0x130520b8
  // extends androidx.fragment.app.FragmentActivity
    [0x125] boolean mStopped = false
    [0x124] boolean mStartedIntentSenderFromFragment = false
    [0x123] boolean mStartedActivityFromFragment = false
    [0x122] boolean mResumed = true
    [0x121] boolean mRequestedPermissionsFromFragment = false
    [0x120] boolean mCreated = true
    [0x11c] int mNextCandidateRequestIndex = 0
    [0x118] androidx.collection.SparseArrayCompat mPendingFragmentActivityResults = 0x13052188
    [0x114] final androidx.fragment.app.FragmentController mFragments = 0x13052178
    [0x110] final androidx.lifecycle.LifecycleRegistry mFragmentLifecycleRegistry = 0x13052150
    ...
```

```
core-parser> help class
Usage: class [CLASSNAME] [OPTION]
Option:
    -m, --method       show class method
    -i, --impl         show class implements class
    -s, --static       show static field
    -f, --field        show instance field
    -x, --hex          basic type hex print

core-parser> class android.net.wifi.WifiNetworkSpecifier
[0x71c530a0]
public final class android.net.wifi.WifiNetworkSpecifier extends android.net.NetworkSpecifier {
  // Implements:
    android.os.Parcelable

  // Class static fields:
    [0x104] private final static java.lang.String TAG = 0x0
    [0x100] public final static android.os.Parcelable$Creator CREATOR = 0x0

  // Object instance fields:
    [0x010] public final android.net.wifi.WifiConfiguration wifiConfiguration
    [0x00c] public final android.os.PatternMatcher ssidPatternMatcher
    [0x008] public final android.util.Pair bssidPatternMatcher

  // extends android.net.NetworkSpecifier

  // extends java.lang.Object
    [0x004] private transient int shadow$_monitor_
    [0x000] private transient java.lang.Class shadow$_klass_

  // Methods:
    [0x791af097e8c8] static void android.net.wifi.WifiNetworkSpecifier.<clinit>()
    [0x791af097e8f0] public void android.net.wifi.WifiNetworkSpecifier.<init>()
    [0x791af097e918] public void android.net.wifi.WifiNetworkSpecifier.<init>(android.os.PatternMatcher, android.util.Pair, android.net.wifi.WifiConfiguration)
    [0x791af097e940] public boolean android.net.wifi.WifiNetworkSpecifier.canBeSatisfiedBy(android.net.NetworkSpecifier)
    [0x791af097e968] public int android.net.wifi.WifiNetworkSpecifier.describeContents()
    [0x791af097e990] public boolean android.net.wifi.WifiNetworkSpecifier.equals(java.lang.Object)
    [0x791af097e9b8] public int android.net.wifi.WifiNetworkSpecifier.hashCode()
    [0x791af097e9e0] public java.lang.String android.net.wifi.WifiNetworkSpecifier.toString()
    [0x791af097ea08] public void android.net.wifi.WifiNetworkSpecifier.writeToParcel(android.os.Parcel, int)

core-parser> class android.net.wifi.WifiNetworkSpecifier -f -s
[0x71c530a0]
public final class android.net.wifi.WifiNetworkSpecifier extends android.net.NetworkSpecifier {
  // Class static fields:
    [0x104] private final static java.lang.String TAG = 0x0
    [0x100] public final static android.os.Parcelable$Creator CREATOR = 0x0

  // Object instance fields:
    [0x010] public final android.net.wifi.WifiConfiguration wifiConfiguration
    [0x00c] public final android.os.PatternMatcher ssidPatternMatcher
    [0x008] public final android.util.Pair bssidPatternMatcher

  // extends android.net.NetworkSpecifier

  // extends java.lang.Object
    [0x004] private transient int shadow$_monitor_
    [0x000] private transient java.lang.Class shadow$_klass_

core-parser> class android.net.wifi.WifiNetworkSpecifier -m | grep desc
    [0x791af097e968] public int android.net.wifi.WifiNetworkSpecifier.describeContents()
```

```
core-parser> help method
Usage: method <ART_METHOD> [OPTIONE...]
Option:
    --dex-dump        show dalvik byte codes
    -i, --inst <PC>   only dex-dump, show instpc byte code
    -n, --num <NUM>   only dex-dump, show maxline num
    --oat-dump        show oat machine codes
        --pc <PC>     only oat-dump
    -b, --binary      show ArtMethod memory
    -v, --verbaose    show more info

core-parser> method 0x70b509c0 -v --dex-dump --oat-dump
public static void com.android.internal.os.ZygoteInit.main(java.lang.String[]) [dex_method_idx=49967]
Location      : /system/framework/framework.jar!classes3.dex
CodeItem      : 0x79185a704704
Registers     : 19
Ins           : 1
Outs          : 4
Insns size    : 0x167
DEX CODE:
  0x79185a704714: 0108 0012                | move-object/from16 v1, v18
  0x79185a704718: 001a 07bc                | const-string v0, "--socket-name=" // string@1980
  0x79185a70471c: 021a 0791                | const-string v2, "--abi-list=" // string@1937
  0x79185a704720: 031a 8897                | const-string v3, "Zygote" // string@34967
  0x79185a704724: 0412                     | const/4 v4, #+0
  0x79185a704726: 0071 e9ef 0000           | invoke-static {}, void dalvik.system.ZygoteHooks.startZygoteNoThreadCreation() // method@59887
  ...
OatQuickMethodHeader(0x719075d8)
  code_offset: 0x719075e0
  code_size: 0x944
  vmap_table_offset: 0x74d2f2
    CodeInfo BitSize=7614 FrameSize:0xb0 CoreSpillMask:0x1f028 FpSpillMask:0x0 NumberOfDexRegisters:19
      StackMap BitSize=3600 Rows=75 Bits={Kind=1 PackedNativePc=0xc DexPc=0x9 RegisterMaskIndex=4 StackMaskIndex=5 InlineInfoIndex=4 DexRegisterMaskIndex=5 DexRegisterMapIndex=8}
      RegisterMask BitSize=247 Rows=13 Bits={Value=15 Shift=4}
      StackMask BitSize=893 Rows=19 Bits={Mask=47}
      InlineInfo BitSize=130 Rows=10 Bits={IsLast=1 DexPc=4 MethodInfoIndex=3 ArtMethodHi=0 ArtMethodLo=0 NumberOfDexRegisters=5}
      MethodInfo BitSize=112 Rows=7 Bits={MethodIndex=16}
      DexRegisterMask BitSize=744 Rows=31 Bits={Mask=24}
      DexRegisterMap BitSize=906 Rows=151 Bits={CatalogueIndex=6}
      DexRegisterInfo BitSize=702 Rows=39 Bits={Kind=3 PackedValue=15}
  QuickMethodFrameInfo
    frame_size_in_bytes: 0xb0
    core_spill_mask: 0x1f028 (rbx, rbp, r12, r13, r14, r15, rip)
    fp_spill_mask: 0x0 
OAT CODE:
  [0x719075e0, 0x71907f24]
  0x719075e0:         ffffe00024848548 | test qword ptr [rsp - 0x2000], rax
    GeneralStackMap[0] (NativePc=0x719075e8 DexPc=0x79185a704714)
  0x719075e8:                     5741 | push r15
  0x719075ea:                     5641 | push r14
  0x719075ec:                     5541 | push r13
  0x719075ee:                     5441 | push r12
  0x719075f0:                       55 | push rbp
  0x719075f1:                       53 | push rbx
  0x719075f2:                 78ec8348 | sub rsp, 0x78
  0x719075f6:                 243c8948 | mov qword ptr [rsp], rdi
  0x719075fa:           000000b824b489 | mov dword ptr [rsp + 0xb8], esi
  0x71907601:     0000000000253c836665 | cmp word ptr gs:[0], 0
  0x7190760b:             00000893850f | jne 0x71907ea4
  0x71907611:                   f38948 | mov rbx, rsi
  0x71907614:             fecf453e2d8d | lea ebp, [rip - 0x130bac2]
  0x7190761a:                 34246c89 | mov dword ptr [rsp + 0x34], ebp
  ...
```

```
core-parser> help dex
Usage: dex [OPTIONE...]
Option:
    -o, --origin           show dex origin name
        --app              dex unpack from app
    -n, --num <NUM>        dex unpack with num
    -d, --dir <DIR_PATH>   unpack output path

core-parser> dex
NUM DEXCACHE    REGION                   FLAGS NAME
  1 0x6f79ca38  [79185c1ac000, 79185c66a000)  r--  /apex/com.android.art/javalib/core-oj.jar [*]
  2 0x6fa28ad8  [791af2506000, 791af25a8000)  r--  /apex/com.android.art/javalib/core-libart.jar [*]
  3 0x6fa838b8  [79185bf30000, 79185c1ac000)  r--  /apex/com.android.art/javalib/core-icu4j.jar [*]
  4 0x6fb4f5e0  [791af2425000, 791af2488000)  r--  /apex/com.android.art/javalib/okhttp.jar [*]
  5 0x6fb86100  [791af2166000, 791af22bd000)  r--  /apex/com.android.art/javalib/bouncycastle.jar [*]
  6 0x6fbca100  [79185be08000, 79185bf30000)  r--  /apex/com.android.art/javalib/apache-xml.jar [*]
  7 0x701332b8  [79185b4dc000, 79185be08000)  r--  /system/framework/framework.jar [*]
  8 0x70133328  [79185aba9000, 79185b4dc000)  r--  /system/framework/framework.jar!classes2.dex [*]
  9 0x70133398  [79185a2b7000, 79185aba9000)  r--  /system/framework/framework.jar!classes3.dex [*]
 10 0x70133408  [79185a193000, 79185a2b7000)  r--  /system/framework/framework.jar!classes4.dex [*]
 ...
 23 0x71c550b0  [791af11b1000, 791af11bf000)  r--  /apex/com.android.tethering/javalib/framework-tethering.jar [*]
 24 0x13055198  [791804f7c000, 791805174000)  r--  /data/app/~~Wsw9iRlteEkzqfH0HmhjZA==/penguin.opencore.tester-ATGDVXhbp2-xRwHf7iCsqQ==/base.apk [*]
 25 0x130b1718  [791848aee000, 791848b21000)  r--  /data/app/~~Wsw9iRlteEkzqfH0HmhjZA==/penguin.opencore.tester-ATGDVXhbp2-xRwHf7iCsqQ==/base.apk!classes2.dex [*]
 26 0x13050b80  [791aedbfc000, 791aedbff000)  r--  /data/app/~~Wsw9iRlteEkzqfH0HmhjZA==/penguin.opencore.tester-ATGDVXhbp2-xRwHf7iCsqQ==/base.apk!classes3.dex [*]

core-parser> dex --app
Saved [./base.apk_0xc65c568b].
Saved [./base.apk!classes2.dex_0x9758703].
Saved [./base.apk!classes3.dex_0x4edd148b].

core-parser> dex -n 7
Saved [./framework.jar_0x347a29fd].
```

```
core-parser> help map
Usage: map [OPTION]
Option:
    -o, --ori         show origin link map
    -s, --sym <NUM>   show link map current symbols

core-parser> map
NUM LINKMAP       REGION                   FLAGS NAME
  1 0x791af2b6d0e0  [5a224127f000, 5a2241282000)  r--  /system/bin/app_process64 [*]
  2 0x791af2dd90e0  [791af2cbd000, 791af2cfd000)  r--  /system/bin/linker64 [*]
  3 0x791af2b6d330  [7ffc73ae7000, 7ffc73ae8000)  r-x  [vdso] [*]
  4 0x791af2b6d580  [791af0e08000, 791af0eb3000)  r--  /system/lib64/libandroid_runtime.so [*]
  ...

core-parser> map --sym 3
VADDR             SIZE              INFO              NAME
00007ffc73ae7a10  000000000000002a  0000000000000022  getcpu
00007ffc73ae77c0  00000000000001de  0000000000000022  clock_gettime
00007ffc73ae77a0  0000000000000015  0000000000000012  __vdso_time
00007ffc73ae7610  000000000000018a  0000000000000022  gettimeofday
00007ffc73ae7a10  000000000000002a  0000000000000012  __vdso_getcpu
00007ffc73ae79a0  0000000000000047  0000000000000022  clock_getres
00007ffc73ae77a0  0000000000000015  0000000000000022  time
00007ffc73ae79a0  0000000000000047  0000000000000012  __vdso_clock_getres
00007ffc73ae77c0  00000000000001de  0000000000000012  __vdso_clock_gettime
00007ffc73ae7610  000000000000018a  0000000000000012  __vdso_gettimeofday
```

```
core-parser> help disas
Usage: disassemble|disas <SYMBOL> [OPTION]
Option:
    --origin    disassemble from corefile
    --mmap      disassemble from file mmap
    --overlay   disassemble form overwirte

core-parser> disas __vdso_getcpu
LIB: [vdso]
__vdso_getcpu:
  0x7ffc73ae7a10:                       55 | push rbp
  0x7ffc73ae7a11:                   e58948 | mov rbp, rsp
  0x7ffc73ae7a14:               0000007bb8 | mov eax, 0x7b
  0x7ffc73ae7a19:                   c0030f | lsl eax, eax
  0x7ffc73ae7a1c:                       90 | nop
  0x7ffc73ae7a1d:                   ff8548 | test rdi, rdi
  0x7ffc73ae7a20:                     0a74 | je 0x7ffc73ae7a2c
  0x7ffc73ae7a22:                     c189 | mov ecx, eax
  0x7ffc73ae7a24:             00000fffe181 | and ecx, 0xfff
  0x7ffc73ae7a2a:                     0f89 | mov dword ptr [rdi], ecx
  0x7ffc73ae7a2c:                   f68548 | test rsi, rsi
  0x7ffc73ae7a2f:                     0574 | je 0x7ffc73ae7a36
  0x7ffc73ae7a31:                   0ce8c1 | shr eax, 0xc
  0x7ffc73ae7a34:                     0689 | mov dword ptr [rsi], eax
  0x7ffc73ae7a36:                     c031 | xor eax, eax
  0x7ffc73ae7a38:                       5d | pop rbp
  0x7ffc73ae7a39:                       c3 | ret
```

```
core-parser> help exec
Usage: exec <EXEC_PATH>

core-parser> exec /system/bin/app_process64
Mmap segment [60969cb26000, 60969cb28000) /system/bin/app_process64 [0]
Mmap segment [60969cb28000, 60969cb2a000) /system/bin/app_process64 [1000]
```

```
core-parser> help sysroot
Usage: sysroot <DIR_PATH>[:<PATH>:<PATH>] [OPTION]
Option:
    --map   set sysroot link_map
    --dex   set sysroot dex_cache

core-parser> sysroot /system:/apex --map
Mmap segment [60969cb26000, 60969cb28000) /system/bin/app_process64 [0]
Mmap segment [60969cb28000, 60969cb2a000) /system/bin/app_process64 [1000]
Mmap segment [75dc65a44000, 75dc65a8b000) /system/bin/linker64 [0]
Mmap segment [75dc65a8b000, 75dc65b8e000) /system/bin/linker64 [46000]
Mmap segment [75dc4d4f8000, 75dc4d5d5000) /system/lib64/libandroid_runtime.so [0]
...

core-parser> sysroot /system:/apex --dex
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
Usage: write|wd <ADDRESS> <OPTION>
Option:
    -s, --string <STRING>   overwrite string at dist address
    -v, --value <VALUE>     overwrite value at dis address

core-parser> p 0x71edf4f8 -b
Size: 0x28
Object Name: java.lang.String
    [0x10] virutal char[] values = "AES128-GCM-SHA256"
    [0x0c] private int hash = -1058959256
    [0x08] private final int count = 17
  // extends java.lang.Object
    [0x04] private transient int shadow$_monitor_ = 536870912
    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f817d58
Binary:
71edf4f8: 200000006f817d58  c0e1906800000022  X}.o...."...h...
71edf508: 472d383231534541  35324148532d4d43  AES128-GCM-SHA25
71edf518: 0000000000000036  200000006f817d58  6.......X}.o....

core-parser> wd 71edf508 -s PenguinLetsGo
New overlay [71bd5000, 71eea000)
core-parser> p 0x71edf4f8 -b
Size: 0x28
Object Name: java.lang.String
    [0x10] virutal char[] values = "PenguinLetsGo"
    [0x0c] private int hash = -1058959256
    [0x08] private final int count = 17
  // extends java.lang.Object
    [0x04] private transient int shadow$_monitor_ = 536870912
    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f817d58
Binary:
71edf4f8: 200000006f817d58  c0e1906800000022  X}.o...."...h...
71edf508: 4c6e6975676e6550  3532006f47737465  PenguinLetsGo.25
71edf518: 0000000000000036  200000006f817d58  6.......X}.o....
```

```
core-parser> help rd
Usage: read|rd <BEGIN_ADDR> [OPTION..]
Priority: overlay > mmap > origin
Option:
    -e, --end <END_ADDR>   read [BEGIN, END) memory content
        --origin           read memory content from corefile
        --mmap             read memory content from file mmap
        --overlay          read memory content form overwirte
    -i, --inst             read memory content convert asm code
    -f, --file <PATH>      read memory binary save to output file

core-parser> rd 75d9a3fa8000 -e 75d9a3fa8020
75d9a3fa8000: 0000000000000000  0202020202020202  ................
75d9a3fa8010: 0202020202020202  0230020202020202  ..............0.

core-parser> rd 75d9a3fa8000 -e 75d9a3fa8020 --origin
75d9a3fa8000: 0202020202020202  0202020202020202  ................
75d9a3fa8010: 0202020202020202  0230020202020202  ..............0.

core-parser> rd 0x71907db5 -e 0x71907ddd -i
0x71907db5:                   2057ff | call qword ptr [rdi + 0x20]
0x71907db8:                     db85 | test ebx, ebx
0x71907dba:             00000018840f | je 0x71907dd8
0x71907dc0:                   de8948 | mov rsi, rbx
0x71907dc3:                     3e8b | mov edi, dword ptr [rsi]
0x71907dc5:               0000ebbeb8 | mov eax, 0xebbe
0x71907dca:           00000080bf8b48 | mov rdi, qword ptr [rdi + 0x80]
0x71907dd1:                 207f8b48 | mov rdi, qword ptr [rdi + 0x20]
0x71907dd5:                   2057ff | call qword ptr [rdi + 0x20]
0x71907dd8:                 78c48348 | add rsp, 0x78
0x71907ddc:                       5b | pop rbx
0x71907ddd:                       5d | pop rbp

core-parser> rd 791804999000 -e 7918049ac000 -f libGLESv2_emulation.so
Saved [libGLESv2_emulation.so].
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
core-parser> help file
Usage: file [ADDRESS]

core-parser> file | grep app_process
[5a224127f000, 5a2241282000)  0000000000000000  /system/bin/app_process64
[5a2241282000, 5a2241286000)  0000000000002000  /system/bin/app_process64
[5a2241286000, 5a2241288000)  0000000000005000  /system/bin/app_process64
[5a2241288000, 5a2241289000)  0000000000006000  /system/bin/app_process64

core-parser> file 0x71907dc5
[71224000, 71b89000)  0000000000203000  /system/framework/x86_64/boot-framework.oat
```

```
core-parser> help bt
Usage: backtrace|bt [PID..] [OPTION]
Option:
    -a, --all           show thread stack.
    -d, --detail        show more info.
        --fp <FP_REG>   only support arm64

core-parser> bt
"main" sysTid=6118 Runnable
  | group="main" daemon=0 prio=5 target=0x0
  | tid=1 sCount=0 flags=0 obj=0x71bdaeb8 self=0x7919cce70380
  | stack=0x7ffc732d1000-0x7ffc732d3000 stackSize=0x800000 handle=0x791af2dde4f8
  | mutexes=0x7919cce70b30 held="mutator lock"(shared held)
  rax 0x0000000000000000  rbx 0x0000000000000000  rcx 0x0000000000000000  rdx 0x0000000000000000
  r8  0x0000000000000002  r9  0x00007919cce70380  r10 0x0000000000000001  r11 0x0000000000000029
  r12 0x000079192ce6b090  r13 0x00007919cce70380  r14 0x0000000000000002  r15 0x0000000070897508
  rdi 0x000079192ce6b090  rsi 0x0000000070d05730
  rbp 0x000000000000f9e1  rsp 0x00007ffc73acc290  rip 0x000079185c88945f  flags 0x0000000000010246
  ds 0x00000000  es 0x00000000  fs 0x00000000  gs 0x00000000  cs 0x00000033  ss 0x0000002b
  Native: #0  000079185c88945f
  JavaKt: #0  000079185af57268  android.os.ThreadLocalWorkSource.getToken
  JavaKt: #1  000079185af57290  android.os.ThreadLocalWorkSource.setUid
  JavaKt: #2  000079185af3fdba  android.os.Looper.loop
  JavaKt: #3  000079185b67d8d6  android.app.ActivityThread.main
  JavaKt: #4  0000000000000000  java.lang.reflect.Method.invoke
  JavaKt: #5  000079185a700626  com.android.internal.os.RuntimeInit$MethodAndArgsCaller.run
  JavaKt: #6  000079185a704980  com.android.internal.os.ZygoteInit.main
```

```
core-parser> help frame
Usage: frame|f <NUM> [OPTION..]
Option:
    -j, --java       show java frame info (default)
    -n, --native     show native frame info
    -a, --all        show all frame info

core-parser> f 6 -j
  JavaKt: #6  000079185a704980  com.android.internal.os.ZygoteInit.main(java.lang.String[])
  {
      Location: /system/framework/framework.jar!classes3.dex
      art::ArtMethod: 0x70b509c0
      dex_pc_ptr: 0x79185a704980
      quick_frame: 0x7ffc73acdc70
      frame_pc: 0x71907dd5
      method_header: 0x719075d8

      DEX CODE:
      0x79185a704976: 106e c34e 0002           | invoke-virtual {v2}, void com.android.internal.os.ZygoteServer.closeServerSocket() // method@49998
      0x79185a70497c: 0038 0005                | if-eqz v0, 0x79185a704986 //+5
      0x79185a704980: 1072 ebbe 0000           | invoke-interface {v0}, void java.lang.Runnable.run() // method@60350

      OAT CODE:
      0x71907db5:                   2057ff | call qword ptr [rdi + 0x20]
      0x71907db8:                     db85 | test ebx, ebx
      0x71907dba:             00000018840f | je 0x71907dd8
      0x71907dc0:                   de8948 | mov rsi, rbx
      0x71907dc3:                     3e8b | mov edi, dword ptr [rsi]
      0x71907dc5:               0000ebbeb8 | mov eax, 0xebbe
      0x71907dca:           00000080bf8b48 | mov rdi, qword ptr [rdi + 0x80]
      0x71907dd1:                 207f8b48 | mov rdi, qword ptr [rdi + 0x20]
      0x71907dd5:                   2057ff | call qword ptr [rdi + 0x20]
      0x71907dd8:                 78c48348 | add rsp, 0x78
      0x71907ddc:                       5b | pop rbx
      0x71907ddd:                       5d | pop rbp
      {
          rcx = 0x0000000012c5a298    rdx = 0x0000000013040218    rbx = 0x0000000000000004    rbp = 0x0000000012c5f340
          rsi = 0x0000000000000000    r8 = 0x000079185a9cb7f1    r9 = 0x0000000071bd5fb8    r12 = 0x00000000705d9d70
          r13 = 0x0000000071be62f0    r14 = 0x0000000000000000    r15 = 0x0000000071907dd8
      }
  }
```

```
core-parser> help regs
Usage: register|regs [TID] [OPTION...]
Option:
    -s, --set <REGS>=<VALUE>    overwrite register value
    -g, --get <REGS>            get target register value

core-parser> regs --set rip=0x00000074e414e2a0
New note overlay [7d6d8, 104073)
```

```
core-parser> help remote
Usage: remote <COMMAND> [OPTION...]
Command:
    core    hook    rd    wd
    pause   setprop

Usage: remote core -p <PID> [-m <MACHINE>] [OPTION...]
Option:
    -p, --pid <PID>           set target pid
    -d, --dir <DIR>           set target dir
    -m, --machine <Machine>   set target machine
Machine:
     { arm64, arm, x86_64, x86, riscv64 }
    -o, --output <COREFILE>   set coredump filename
    -f, --filter <Filter>     set coredump ignore filter
Filter: (0x19 default)
     0x01: filter-special-vma (default)
     0x02: filter-file-vma
     0x04: filter-shared-vma
     0x08: filter-sanitizer-shadow-vma (default)
     0x10: filter-non-read-vma (default)

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

remote wd -p <PID> <ADDRESS> [-s|-v] <VALUE>
core-parser> remote wd -p 1 7fb989794000 -s PenguinLetsGo

remote rd -p <PID> <BEGIN_ADDR> -e <END_ADDR>
core-parser> remote rd -p 1 7fb989794000 -e 7fb989794030
7fb989794000: 4c6e6975676e6550  0000006f47737465  PenguinLetsGo...
7fb989794010: 00000001003e0003  0000000000068ab0  ..>.............
7fb989794020: 0000000000000040  0000000000198c20  @...............

remote pause <PID ...> [-a]
```

```
core-parser> help fake
Usage: fake <COMMAND> [OPTION...]
Command:
    core    map    stack

Usage: fake core <OPTION...>
Option:
    -t, --tomb <TOMBSTONE>    build tombstone fakecore
    -r, --rebuild             rebuild current environment core.
    -o, --output <COREFILE>   set current fakecore path

core-parser> fake core -r
FakeCore: saved [core.opencore.tester_6118_Thread-2_6146_1720691326.fakecore]

Usage: fake map

Usage: fake stack --pc <PC> --sp <SP> [OPTION]
Option:
    -c, --clean    clean fake java stack pc, sp
```

```
core-parser> help env
Usage: env <COMMAND> [OPTION] ...
Command:
    config  logger  art  core

Usage: env config <OPTION> ..
Option:
        --sdk <VERSION>   set current sdk version
        --oat <VERSION>   set current oat version
    -p, --pid <PID>       set current thread

core-parser> env config --sdk 30
Switch android(30) env.

Usage: env logger <LEVEL>
Level:
        --debug           set current logger level to debug
        --info            set current logger level to info
        --warn            set current logger level to warn
        --error           set current logger level to error
        --fatal           set current logger level to fatal

core-parser> env logger
Current logger level error

Usage: env art [OPTION] ...
Option:
    -c, --clean-cache     clean art::Runtime cache
    -e, --entry-points    show art quick entry points
    -n, --nterp           show art nterp cache

core-parser> env art
  * LIB: /apex/com.android.art/lib64/libart.so
  * art::OatHeader::kOatVersion: 183
  * art::Runtime: 0x79196ce69360
  * art::gc::Heap: 0x79196ce6d3c0
  *     continuous_spaces_: 0x79196ce6d3c0
  *     discontinuous_spaces_: 0x79196ce6d3d8
  * art::MonitorPool: 0x7918cce64ae0
  * art::ThreadList: 0x7919dce69430
  *     list_: 0x7919dce6b430
  * art::ClassLinker: 0x79192ce6b090
  *     dex_caches_: 0x79192ce6b0c8
  * art::JavaVMExt: 0x79192ce68310
  *     globals_: 0x79192ce68350
  *     weak_globals_: 0x79192ce683d8
  * art::jit::Jit: 0x79193ce73a70
  *     code_cache_: 0x79196ce6ad20

Usage: env core [OPTION]...
Option:
        --load            show corefile load segments
        --quick-load      show corefile quick load segments
        --arm <thumb|arm> set arm disassemble mode
        --crc             check consistency of mmap file data
    -c, --clean-cache     clean link_map cache

core-parser> env core
  * r_debug: 0x791af2dd7bf0
  * arm mode: thumb
  * mLoad: 1985
  * mQuickLoad: 1802
  * mLinkMap: 271
```

```
core-parser> help logcat
Usage: logcat [OPTION]...
Option:
    -b, --buffer <BUFFER>  collect only from buffers
Buffer:{main, radio, events, system, crash, kernel}
    -p, --pid <PID>        collect only from pid
    -u, --uid <UID>        collect only from uid
    -t, --tid <TID>        collect only from tid

core-parser> logcat -b crash -p 11770
--------- beginning of crash
2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: FATAL EXCEPTION: FinalizerWatchdogDaemon
2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: Process: com.demo.app, PID: 11770
2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: java.util.concurrent.TimeoutException: android.content.res.ApkAssets.finalize() timed out after 40 seconds
2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Daemons$Daemon.isRunning(Unknown Source:0)
2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Daemons$FinalizerDaemon.runInternal(Daemons.java:286)
2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Daemons$Daemon.run(Daemons.java:140)
2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Thread.run(Thread.java:1012)
```

```
core-parser> help getprop
Usage: getprop [NAME]
core-parser> getprop ro.build.description
sdk_gphone_x86_64-userdebug 11 RSR1.210722.013.A2 10067904 dev-keys
```

```
core-parser> help cxx
Usage: cxx <TYPE> <ADDR> [OPTION]
Type:
    string          vector    map
    unordered_map   list      deque
Option:
    -e, --entry-size    only list set entry-size
    -b, block-size      only deque set block-size

core-parser> cxx string 0x79191ce66ed8
/apex/com.android.art/javalib/x86_64/boot-okhttp.art

core-parser> cxx list 0x7919dce6b430 --entry-size 8
[0] 0x79187ce8b9e0
[1] 0x79187cea2450
[2] 0x79187ce9d6e0
[3] 0x79187ceb0580
[4] 0x79187ceac3b0
[5] 0x79187ce8c940
[6] 0x79187ce984f0
[7] 0x79187ceac1a0
[8] 0x79187ceac830
[9] 0x79187ceba570
[10] 0x79187cea1400
[11] 0x79187ceab7e0
[12] 0x79187ce9e7f0
[13] 0x79187ceab6f0
[14] 0x79187ce99ed0
[15] 0x79187ceabbd0
[16] 0x79187ceaefc0
[17] 0x79187ceaf590
[18] 0x79187ceaed50
[19] 0x79187ce8bc80
```

# Plugin
```
core-parser> help plugin
Usage: plugin <PATH> [Option]
Option:
    -u, --unload   remove extend library
    -r, --reload   reload extend library

core-parser> plugin plugin-simple.so
Linker env...
env new command "simple"
core-parser> simple
command simple!
```
