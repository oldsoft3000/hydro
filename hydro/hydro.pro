INCLUDEPATH += /home/oleg/devel/libs/Box2D/Box2D
INCLUDEPATH += ../shared
INCLUDEPATH += ../shared/measures
INCLUDEPATH += ../shared/math
INCLUDEPATH += ../shared/physycs
INCLUDEPATH += tools
INCLUDEPATH += geometry
INCLUDEPATH += ../shared/poly2tri
INCLUDEPATH += ../shared/delaunay-triangulation

CONFIG += c++11

linux:!android {
    message("* Using settings for Unix/Linux.")
    LIBS += -L$$PWD/../libs/linux/Box2D/Box2D/Build/Box2D -lBox2D

}

android {
    message("* Using settings for Android.")
    equals(ANDROID_TARGET_ARCH, armeabi-v7a) {
        LIBS += -L$$PWD/../libs/android/Box2D/libs/armeabi-v7a -lbox2d
    }
    equals(ANDROID_TARGET_ARCH, armeabi) {
        LIBS += -L$$PWD/../libs/android/Box2D/libs/armeabi -lbox2d
    }
    equals(ANDROID_TARGET_ARCH, x86)  {
        LIBS += -L$$PWD/../libs/android/Box2D/libs/x86 -lbox2d
    }
}



SOURCES += \
    main.cpp \
    WorldView.cpp \
    WorldController.cpp \
    ../shared/physycs/ElementBase.cpp \
    ../shared/physycs/PhysicsController.cpp \
    Atom.cpp \
    Fluid.cpp \
    ObjectBase.cpp \
    FixturePainter.cpp \
    FixturePainterGl.cpp \
    geometry/GeometryEngine.cpp \
    ../shared/math/GeometryUtils.cpp \
    ../shared/physycs/ElementUtils.cpp \
    ../shared/math/MathUtils.cpp \
    ../shared/CommonUtils.cpp \
    DebugPanel.cpp \
    Game.cpp \
    ../shared/measures/MeasureStats.cpp \
    ../shared/measures/Ticks.cpp \
    ../shared/math/Interpolator.cpp \
    FluidParams.cpp \
    geometry/FluidGeometry.cpp \
    geometry/SceneGeometry.cpp \
    geometry/SnippetArea.cpp \
    unit_tests/FluidGeometryTests.cpp \
    ../shared/math/Triangulate.cpp \
    geometry/FluidTriangles.cpp \
    geometry/FluidGeometryUtils.cpp \
    ../shared/poly2tri/common/shapes.cc \
    ../shared/poly2tri/sweep/advancing_front.cc \
    ../shared/poly2tri/sweep/cdt.cc \
    ../shared/poly2tri/sweep/sweep.cc \
    ../shared/poly2tri/sweep/sweep_context.cc \
    geometry/ColorGenerator.cpp \
    GlobalParams.cpp

QT += opengl

HEADERS += \
    WorldView.h \
    WorldController.h \
    ../shared/physycs/PhysicsController.h \
    ../shared/physycs/ElementBase.h \
    Atom.h \
    Fluid.h \
    ObjectBase.h \
    FixturePainter.h \
    FixturePainterGl.h \
    geometry/GeometryEngine.h \
    ../shared/math/GeometryUtils.h \
    Types.h \
    ../shared/physycs/ElementUtils.h \
    ../shared/math/MathUtils.h \
    ../shared/CommonUtils.h \
    DebugPanel.h \
    Game.h \
    ../shared/measures/MeasureStats.h \
    ../shared/measures//Ticks.h \
    ../shared/math/LinearSpeedInterpolator.h \
    ../shared/math/Interpolator.h \
    FluidParams.h \
    geometry/FluidGeometry.h \
    geometry/SceneGeometry.h \
    geometry/SnippetArea.h \
    unit_tests/FluidGeometryTests.h \
    ../shared/math/Triangulate.h \
    geometry/FluidTriangles.h \
    geometry/FluidGeometryUtils.h \
    ../shared/poly2tri/common/shapes.h \
    ../shared/poly2tri/common/utils.h \
    ../shared/poly2tri/sweep/advancing_front.h \
    ../shared/poly2tri/sweep/cdt.h \
    ../shared/poly2tri/sweep/sweep.h \
    ../shared/poly2tri/sweep/sweep_context.h \
    ../shared/poly2tri/poly2tri.h \
    ../shared/delaunay-triangulation/delaunay.h \
    ../shared/delaunay-triangulation/edge.h \
    ../shared/delaunay-triangulation/triangle.h \
    ../shared/delaunay-triangulation/vector2.h \
    geometry/ColorGenerator.h \
    GlobalParams.h

RESOURCES += \
    shaders.qrc



contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../libs/android/Box2D/libs/x86/libbox2d.so
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/../libs/android/Box2D/libs/armeabi-v7a/libbox2d.so
}


QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3
QMAKE_CXXFLAGS += -fno-rtti

//QMAKE_CXX = DISTCC_LOG=/home/oleg/distcc.log distcc x86_64-pc-linux-gnu-g++-4.9.3

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
