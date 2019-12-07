/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QWidget *centralWidget;
    QPushButton *openButton;
    QLabel *ImgDisplay;
    QPushButton *saveButton;
    QPushButton *resetButton;
    QPushButton *bwButton;
    QSlider *zoomSlider;
    QLabel *label;
    QScrollBar *horizontalScrollBar;
    QScrollBar *verticalScrollBar;
    QPushButton *saveDisplayButton;
    QPushButton *halfButton;
    QFrame *line;
    QSlider *brightnessSlider;
    QLabel *label_14;
    QPushButton *rowleyButton;
    QPushButton *oCV_Button;
    QLabel *label_15;
    QLabel *label_16;
    QSpinBox *randomNumBox;
    QStatusBar *statusBar;
    QToolBar *mainToolBar;
    QMenuBar *menuBar;
    QMenu *menuFile;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(996, 693);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        openButton = new QPushButton(centralWidget);
        openButton->setObjectName(QString::fromUtf8("openButton"));
        openButton->setGeometry(QRect(20, 40, 121, 21));
        ImgDisplay = new QLabel(centralWidget);
        ImgDisplay->setObjectName(QString::fromUtf8("ImgDisplay"));
        ImgDisplay->setGeometry(QRect(330, 19, 640, 521));
        saveButton = new QPushButton(centralWidget);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));
        saveButton->setGeometry(QRect(20, 70, 121, 21));
        resetButton = new QPushButton(centralWidget);
        resetButton->setObjectName(QString::fromUtf8("resetButton"));
        resetButton->setGeometry(QRect(170, 40, 131, 23));
        bwButton = new QPushButton(centralWidget);
        bwButton->setObjectName(QString::fromUtf8("bwButton"));
        bwButton->setGeometry(QRect(170, 70, 131, 23));
        zoomSlider = new QSlider(centralWidget);
        zoomSlider->setObjectName(QString::fromUtf8("zoomSlider"));
        zoomSlider->setGeometry(QRect(40, 600, 291, 19));
        zoomSlider->setMaximum(999);
        zoomSlider->setOrientation(Qt::Horizontal);
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 600, 31, 16));
        horizontalScrollBar = new QScrollBar(centralWidget);
        horizontalScrollBar->setObjectName(QString::fromUtf8("horizontalScrollBar"));
        horizontalScrollBar->setGeometry(QRect(330, 540, 641, 16));
        horizontalScrollBar->setMaximum(999);
        horizontalScrollBar->setOrientation(Qt::Horizontal);
        verticalScrollBar = new QScrollBar(centralWidget);
        verticalScrollBar->setObjectName(QString::fromUtf8("verticalScrollBar"));
        verticalScrollBar->setGeometry(QRect(970, 20, 20, 521));
        verticalScrollBar->setMaximum(999);
        verticalScrollBar->setOrientation(Qt::Vertical);
        saveDisplayButton = new QPushButton(centralWidget);
        saveDisplayButton->setObjectName(QString::fromUtf8("saveDisplayButton"));
        saveDisplayButton->setGeometry(QRect(20, 100, 121, 21));
        halfButton = new QPushButton(centralWidget);
        halfButton->setObjectName(QString::fromUtf8("halfButton"));
        halfButton->setGeometry(QRect(170, 100, 131, 23));
        line = new QFrame(centralWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(310, 10, 20, 551));
        line->setLineWidth(2);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        brightnessSlider = new QSlider(centralWidget);
        brightnessSlider->setObjectName(QString::fromUtf8("brightnessSlider"));
        brightnessSlider->setGeometry(QRect(430, 600, 271, 19));
        brightnessSlider->setMaximum(999);
        brightnessSlider->setOrientation(Qt::Horizontal);
        label_14 = new QLabel(centralWidget);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(390, 600, 31, 16));
        rowleyButton = new QPushButton(centralWidget);
        rowleyButton->setObjectName(QString::fromUtf8("rowleyButton"));
        rowleyButton->setGeometry(QRect(110, 190, 151, 23));
        oCV_Button = new QPushButton(centralWidget);
        oCV_Button->setObjectName(QString::fromUtf8("oCV_Button"));
        oCV_Button->setGeometry(QRect(110, 220, 151, 23));
        label_15 = new QLabel(centralWidget);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(10, 190, 91, 21));
        label_16 = new QLabel(centralWidget);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setGeometry(QRect(10, 290, 91, 21));
        QFont font;
        font.setPointSize(7);
        label_16->setFont(font);
        randomNumBox = new QSpinBox(centralWidget);
        randomNumBox->setObjectName(QString::fromUtf8("randomNumBox"));
        randomNumBox->setGeometry(QRect(110, 290, 151, 21));
        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 996, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionOpen);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
        openButton->setText(QCoreApplication::translate("MainWindow", "Open Image", nullptr));
        ImgDisplay->setText(QString());
        saveButton->setText(QCoreApplication::translate("MainWindow", "Save Image", nullptr));
        resetButton->setText(QCoreApplication::translate("MainWindow", "Reset", nullptr));
        bwButton->setText(QCoreApplication::translate("MainWindow", "B/W", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Zoom", nullptr));
        saveDisplayButton->setText(QCoreApplication::translate("MainWindow", "Save Display", nullptr));
        halfButton->setText(QCoreApplication::translate("MainWindow", "Half size", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow", "Gain", nullptr));
        rowleyButton->setText(QCoreApplication::translate("MainWindow", "Rowley Face Detection", nullptr));
        oCV_Button->setText(QCoreApplication::translate("MainWindow", "openCV Face Detection", nullptr));
        label_15->setText(QCoreApplication::translate("MainWindow", "N N Face Detection:", nullptr));
        label_16->setText(QCoreApplication::translate("MainWindow", "Any parameters?", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
