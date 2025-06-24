#-------------------------------------------------
#
# Project created by QtCreator 2023-03-17T21:54:20
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "Markdown Assistant"
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

# msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8

SOURCES += \
    about_dialog.cpp \
    assets_data.cpp \
    assets_type_item.cpp \
    conf_data.cpp \
    create_markdown_dir.cpp \
    file_operation.cpp \
    get_assets_dialog.cpp \
    get_markdown_dialog.cpp \
        main.cpp \
        mainwindow.cpp \
    modify_conf_dialog.cpp \
    open_ex_program.cpp \
    rename_file_name.cpp \
    video_thr.cpp \
    debug_box.cpp

HEADERS += \
    about_dialog.h \
    assets_data.h \
    assets_type_item.h \
    conf_data.h \
    create_markdown_dir.h \
    file_operation.h \
    get_assets_dialog.h \
    get_markdown_dialog.h \
        mainwindow.h \
    modify_conf_dialog.h \
    open_ex_program.h \
    rename_file_name.h \
    video_thr.h \
    data_type.h \
    debug_box.h

FORMS += \
        about_dialog.ui \
        assets_type_item.ui \
        create_markdown_dir.ui \
        get_assets_dialog.ui \
        get_markdown_dialog.ui \
        mainwindow.ui \
        modify_conf_dialog.ui \
        rename_file_name.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc


RC_ICONS = "./qss/icon/markdown-assistant.ico"

# 仅在 macOS 上设置图标
macx {
    RC_ICONS = "qrc:/qss/icon/markdown-assistant.ico"
    MACOSX_BUNDLE_ICON = "qrc:/qss/icon/markdown-assistant.ico"
}


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg-shared/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg-shared/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
else:unix: LIBS += -L$$PWD/ffmpeg-shared/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg-shared/bin -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg-shared/bin -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
else:unix: LIBS += -L$$PWD/ffmpeg-shared/bin -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

INCLUDEPATH += $$PWD/ffmpeg-shared/include
DEPENDPATH += $$PWD/ffmpeg-shared/include

DISTFILES +=


# 设定 FFmpeg 的路径
# 你可以通过 `brew info ffmpeg` 找到正确的安装路径
FFMPEG_PATH = $$PWD/ffmpeg-mac/7.1.1_2

# 包含 FFmpeg 的头文件
INCLUDEPATH += $$FFMPEG_PATH/include

# 链接 FFmpeg 的库
LIBS += -L$$FFMPEG_PATH/lib \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lavfilter \
        -lavdevice
