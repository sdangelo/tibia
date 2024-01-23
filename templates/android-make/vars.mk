BUNDLE_NAME := {{=it.product.bundleName}}
JAVA_PACKAGE_NAME := {{=it.android.javaPackageName}}

KEY_STORE := {{=it.android_make.keyStore}}
KEY_ALIAS := {{=it.android_make.keyAlias}}
STORE_PASS := {{=it.android_make.storePass}}
KEY_PASS := {{=it.android_make.keyPass}}

ANDROID_SDK_DIR := {{=it.android_make.sdkDir}}
BUILD_TOOLS_DIR := ${ANDROID_SDK_DIR}/build-tools/{{=it.android_make.buildToolsVersion}}
ANDROIDX_DIR := {{=it.android_make.androidxDir}}
KOTLIN_DIR := {{=it.android_make.kotlinDir}}

ANDROID_JAR_FILE := ${ANDROID_SDK_DIR}/platforms/android-{{=it.android_make.androidVersion}}/android.jar
ANDROIDX_CORE_FILE := ${ANDROIDX_DIR}/core-{{=it.android_make.androidxCoreVersion}}.jar
ANDROIDX_LIFECYCLE_COMMON_FILE := ${ANDROIDX_DIR}/lifecycle-common-{{=it.android_make.androidxLifecycleCommonVersion}}.jar
ANDROIDX_VERSIONEDPARCELABLE_FILE := ${ANDROIDX_DIR}/versionedparcelable-{{=it.android_make.androidxVersionedparcelableVersion}}.jar
KOTLIN_STDLIB_FILE := ${KOTLIN_DIR}/kotlin-stdlib-{{=it.android_make.kotlinStdlibVersion}}.jar
KOTLINX_COROUTINES_CORE_FILE := ${KOTLIN_DIR}/kotlinx-coroutines-core-{{=it.android_make.kotlinxCoroutinesCoreVersion}}.jar
KOTLINX_COROUTINES_CORE_JVM_FILE := ${KOTLIN_DIR}/kotlinx-coroutines-core-jvm-{{=it.android_make.kotlinxCoroutinesCoreJVMVersion}}.jar

HAS_MIDI_IN := {{=it.product.buses.filter(x => x.type == "midi" && x.direction == "input").length > 0 ? "yes" : "no"}}
