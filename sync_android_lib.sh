#!/usr/bin/env bash

TARGET_DIR='/Users/mac/workspace/build_android'
SRC_DIR='/Users/mac/Project/Urho3D/build_android'

SRC_LIB=${SRC_DIR}/libs/armeabi-v7a/libUrho3DPlayer.so

rm ${TARGET_DIR}/app/build/outputs/apk/debug/app-debug.apk
rm ${TARGET_DIR}/app/src/main/jniLibs/armeabi-v7a/libUrho3DPlayer.so
cp ${SRC_LIB} ${TARGET_DIR}/app/src/main/jniLibs/armeabi-v7a/libUrho3DPlayer.so

sync