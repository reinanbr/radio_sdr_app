# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile

# Keep USB and SDR related classes
-keep class com.sdrradio.app.utils.** { *; }
-keep class com.sdrradio.app.service.** { *; }
-keep class com.sdrradio.app.receiver.** { *; }

# Keep MPAndroidChart classes
-keep class com.github.mikephil.charting.** { *; }

# Keep USB serial library
-keep class com.hoho.android.usbserial.** { *; } 