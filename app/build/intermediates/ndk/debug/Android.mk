LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := app
LOCAL_LDFLAGS := -Wl,--build-id
LOCAL_SRC_FILES := \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/Android.mk \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/Makefile.mgw \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/monster.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/hydra.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/achieve.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/level.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/tutorial.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/mainmenu.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/drawhydras.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/ui.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/classes.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/data.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/utils.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/save.cpp \
	/home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni/hydroid/weapons.cpp \

LOCAL_C_INCLUDES += /home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/debug/jni
LOCAL_C_INCLUDES += /home/vkovun/AndroidStudioProjects/hydroid_AS/app/src/main/jni

include $(BUILD_SHARED_LIBRARY)
