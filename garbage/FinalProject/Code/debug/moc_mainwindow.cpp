/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.13.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.13.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[19];
    char stringdata0[221];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 9), // "OpenImage"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 9), // "SaveImage"
QT_MOC_LITERAL(4, 32, 16), // "SaveDisplayImage"
QT_MOC_LITERAL(5, 49, 10), // "ResetImage"
QT_MOC_LITERAL(6, 60, 11), // "ToggleImage"
QT_MOC_LITERAL(7, 72, 15), // "BlackWhiteImage"
QT_MOC_LITERAL(8, 88, 9), // "HalfImage"
QT_MOC_LITERAL(9, 98, 10), // "Brightness"
QT_MOC_LITERAL(10, 109, 3), // "val"
QT_MOC_LITERAL(11, 113, 4), // "Zoom"
QT_MOC_LITERAL(12, 118, 6), // "Scroll"
QT_MOC_LITERAL(13, 125, 19), // "RowleyFaceDetection"
QT_MOC_LITERAL(14, 145, 19), // "GoogleFaceDetection"
QT_MOC_LITERAL(15, 165, 20), // "ConvertQImage2Double"
QT_MOC_LITERAL(16, 186, 5), // "image"
QT_MOC_LITERAL(17, 192, 20), // "ConvertDouble2QImage"
QT_MOC_LITERAL(18, 213, 7) // "QImage*"

    },
    "MainWindow\0OpenImage\0\0SaveImage\0"
    "SaveDisplayImage\0ResetImage\0ToggleImage\0"
    "BlackWhiteImage\0HalfImage\0Brightness\0"
    "val\0Zoom\0Scroll\0RowleyFaceDetection\0"
    "GoogleFaceDetection\0ConvertQImage2Double\0"
    "image\0ConvertDouble2QImage\0QImage*"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x08 /* Private */,
       3,    0,   85,    2, 0x08 /* Private */,
       4,    0,   86,    2, 0x08 /* Private */,
       5,    0,   87,    2, 0x08 /* Private */,
       6,    0,   88,    2, 0x08 /* Private */,
       7,    0,   89,    2, 0x08 /* Private */,
       8,    0,   90,    2, 0x08 /* Private */,
       9,    1,   91,    2, 0x08 /* Private */,
      11,    1,   94,    2, 0x08 /* Private */,
      12,    1,   97,    2, 0x08 /* Private */,
      13,    0,  100,    2, 0x08 /* Private */,
      14,    0,  101,    2, 0x08 /* Private */,
      15,    1,  102,    2, 0x08 /* Private */,
      17,    1,  105,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QImage,   16,
    QMetaType::Void, 0x80000000 | 18,   16,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OpenImage(); break;
        case 1: _t->SaveImage(); break;
        case 2: _t->SaveDisplayImage(); break;
        case 3: _t->ResetImage(); break;
        case 4: _t->ToggleImage(); break;
        case 5: _t->BlackWhiteImage(); break;
        case 6: _t->HalfImage(); break;
        case 7: _t->Brightness((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->Zoom((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->Scroll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->RowleyFaceDetection(); break;
        case 11: _t->GoogleFaceDetection(); break;
        case 12: _t->ConvertQImage2Double((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 13: _t->ConvertDouble2QImage((*reinterpret_cast< QImage*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
