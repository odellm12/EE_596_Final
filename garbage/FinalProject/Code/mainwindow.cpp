#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QFileDialog>

static const QSize resultSize(640, 480);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(OpenImage()));
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(SaveImage()));
    connect(ui->saveDisplayButton, SIGNAL(clicked()), this, SLOT(SaveDisplayImage()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(ResetImage()));

    connect(ui->bwButton, SIGNAL(clicked()), this, SLOT(BlackWhiteImage()));
    connect(ui->halfButton, SIGNAL(clicked()), this, SLOT(HalfImage()));
    connect(ui->rowleyButton, SIGNAL(clicked()), this, SLOT(RowleyFaceDetection()));
    connect(ui->oCV_Button, SIGNAL(clicked()), this, SLOT(oCVDetection()));
    //connect(ui->histogramButton, SIGNAL(clicked()), this, SLOT(HistogramSeedImage()));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(OpenImage()));
    connect(ui->zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(Zoom(int)));
    connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(Brightness(int)));
    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(Scroll(int)));
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(Scroll(int)));
    ui->zoomSlider->setValue(0);
    ui->brightnessSlider->setValue(0);
    ui->randomNumBox->setValue(4);

    displayImage = QImage(ui->ImgDisplay->width(), ui->ImgDisplay->height(), QImage::Format_RGB32);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DrawDisplayImage()
{
    int zoom = ui->zoomSlider->value()/20 + 1;
    double brightness = (double) ui->brightnessSlider->value()/200.0 + 1.0;
    int r, c;
    int wd = displayImage.width();
    int hd = displayImage.height();
    int ws = outImage.width();
    int hs = outImage.height();

    QRgb pixel;
    pixel = qRgb(240, 240, 240);
    displayImage.fill(pixel);
    double rgb[3];

    for(r=0;r<hd;r++)
    {
        QRgb *scanLine = reinterpret_cast<QRgb *>(displayImage.scanLine(r));
        int rs = (int) ((double) (r - hd/2)/ (double) zoom + yScroll);

        if(rs >= 0 && rs < hs)
        {
            QRgb *scanLineS;

            scanLineS = reinterpret_cast<QRgb *>(outImage.scanLine(rs));

            for(c=0;c<wd;c++)
            {
                int cs = (int) ((double) (c - wd/2)/ (double) zoom + xScroll);

                if(cs >= 0 && cs < ws)
                {
                    pixel = scanLineS[cs];
                    rgb[0] = qRed(pixel);
                    rgb[1] = qGreen(pixel);
                    rgb[2] = qBlue(pixel);

                    rgb[0] *= brightness;
                    rgb[1] *= brightness;
                    rgb[2] *= brightness;

                    rgb[0] = min(255.0, rgb[0]);
                    rgb[1] = min(255.0, rgb[1]);
                    rgb[2] = min(255.0, rgb[2]);

                    scanLine[c] = (uint) qRgb((int) rgb[0], (int) rgb[1], (int) rgb[2]);
                }
            }
        }
    }

    ui->ImgDisplay->setPixmap(QPixmap::fromImage(displayImage));

}

void MainWindow::Zoom(int val)
{
    int zoom = val/20 + 1;
    double c0 = (double) (ui->ImgDisplay->width()/(2*zoom));
    double c1 = (double) (outImage.width()) - c0;


    if(c0 < c1)
    {
        xScroll = min(c1, max(c0, xScroll));
        double newScrollX = (xScroll - c0)/(c1 - c0);
        newScrollX *= (double) ui->horizontalScrollBar->maximum();

        zoomChanged = true;
        ui->horizontalScrollBar->setVisible(true);
        ui->horizontalScrollBar->setValue((int) newScrollX);
    }
    else
    {
        xScroll = (double) outImage.width()/2.0;
        ui->horizontalScrollBar->setVisible(false);
    }

    double r0 = (double) (ui->ImgDisplay->height()/(2*zoom));
    double r1 = (double) (outImage.height()) - r0;

    if(r0 < r1)
    {
        yScroll = min(r1, max(r0, yScroll));
        double newScrollY = (yScroll - r0)/(r1 - r0);
        newScrollY *= (double) ui->verticalScrollBar->maximum();

        zoomChanged = true;
        ui->verticalScrollBar->setVisible(true);
        ui->verticalScrollBar->setValue((int) newScrollY);
    }
    else
    {
        yScroll = (double) outImage.height()/2.0;
        ui->verticalScrollBar->setVisible(false);
    }

    DrawDisplayImage();
}

void MainWindow::Brightness(int val)
{
    DrawDisplayImage();
}

void MainWindow::Scroll(int val)
{
    if(zoomChanged == false)
    {
        int zoom = ui->zoomSlider->value()/20 + 1;
        double c0 = (double) (ui->ImgDisplay->width()/(2*zoom));
        double c1 = (double) (outImage.width()) - c0;

        if(c0 < c1)
        {
            double del = (double)  ui->horizontalScrollBar->value()/ (double) ui->horizontalScrollBar->maximum();
            xScroll = del*(c1 - c0) + c0;
        }
        else
            xScroll = (double) outImage.width()/2.0;

        double r0 = (double) (ui->ImgDisplay->height()/(2*zoom));
        double r1 = (double) (outImage.height()) - r0;

        if(r0 < r1)
        {
            double del = (double)  ui->verticalScrollBar->value()/ (double) ui->verticalScrollBar->maximum();
            yScroll = del*(r1 - r0) + r0;
        }
        else
            yScroll = (double) outImage.height()/2.0;
    }

    zoomChanged = false;

    DrawDisplayImage();
}

void MainWindow::OpenImage()
{
    const QString title;

    QString fileName = QFileDialog::getOpenFileName(this, title);

    if (!fileName.isEmpty())
        inImage.load(fileName);

    outImage = inImage.copy();
    xScroll = (double) outImage.width()/2.0;
    yScroll = (double) outImage.height()/2.0;
    ui->zoomSlider->setValue(0);

    DrawDisplayImage();

}

void MainWindow::SaveImage()
{
    const QString title;

    QString fileName = QFileDialog::getSaveFileName(this, title);

    if (!fileName.isEmpty())
        outImage.save(fileName);
}

void MainWindow::SaveDisplayImage()
{
    const QString title;

    QString fileName = QFileDialog::getSaveFileName(this, title);

    if (!fileName.isEmpty())
        displayImage.save(fileName);
}

void MainWindow::ResetImage()
{
    int w = outImage.width();

    outImage = inImage.copy();

    if(w != outImage.width())
    {
        xScroll = (double) outImage.width()/2.0;
        yScroll = (double) outImage.height()/2.0;
        ui->zoomSlider->setValue(1);
    }

    DrawDisplayImage();
}

void MainWindow::ToggleImage()
{
    DrawDisplayImage();
}


void MainWindow::BlackWhiteImage()
{
    BlackWhiteImage(&outImage);

    DrawDisplayImage();
}


void MainWindow::RowleyFaceDetection()
{
    int num = ui->randomNumBox->value();

    ConvertQImage2Double(outImage);
    RowleyFaceDetection(Image, num);
    ConvertDouble2QImage(&outImage);

    DrawDisplayImage();

}
void MainWindow::oCVDetection()
{
     int num = ui->randomNumBox->value();

     ConvertQImage2Double(outImage);
     oCVDetection(Image, num);
     ConvertDouble2QImage(&outImage);

     DrawDisplayImage();
}

void MainWindow::HalfImage()
{
    HalfImage(outImage);

    xScroll = (double) outImage.width()/2.0;
    yScroll = (double) outImage.height()/2.0;
    ui->zoomSlider->setValue(1);

    DrawDisplayImage();
}

