#!/bin/bash

cd android/Box2D/jni
ndk-build clean
ndk-build NDK_DEBUG=0

