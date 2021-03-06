#ifndef DEFINES_H
#define DEFINES_H

#ifdef Q_OS_WIN
#define APP_NAME        "fourchan-dl.exe"
#if QT_VERSION < 0x050000
#define UPDATE_TREE     "win32"
#else
#define UPDATE_TREE     "win32-qt5"
#endif
#define USE_UPDATER
#define CONSOLE_APPNAME "fourchan-dl-console.exe"
#else
#ifdef Q_OS_LINUX
#define APP_NAME        "fourchan-dl"
#define UPDATE_TREE     "linux"
//#define UPDATER_NAME    "au"
//#define USE_UPDATER
#define CONSOLE_APPNAME "fourchan-dl-console"
#else
#define APP_NAME        ""
#define UPDATE_TREE     "source"
#define CONSOLE_APPNAME "fourchan-dl-console"
#endif
#endif

#define debug_out(STRING, LEVEL)    qDebug() << STRING;

#define __IMAGE_REGEXP__ "(\\.jpg|\\.gif|\\.png|\\.jpeg|\\.webm)+"
#define __IMAGEFILE_REGEXP__ QString("%1%2").arg("\\/(\\w+)").arg(__IMAGE_REGEXP__)

#endif // DEFINES_H
