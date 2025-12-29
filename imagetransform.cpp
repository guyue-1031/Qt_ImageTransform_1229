#include "imagetransform.h"
#include <QFileDialog>
#include <QPixmap>
#include <QTransform>
#include <QtMath>
#include <QVBoxLayout>

ImageTransform::ImageTransform(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("影像處理"));

    fileMenu = menuBar()->addMenu(QStringLiteral("檔案F"));
    toolMenu = menuBar()->addMenu(QStringLiteral("工具T"));

    openAction = new QAction(QStringLiteral("開啟檔案"), this);
    saveAction = new QAction(QStringLiteral("儲存圖片"), this);

    zoomOutAction = new QAction(QStringLiteral("縮小"), this);
    zoomInAction  = new QAction(QStringLiteral("放大"), this);

    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);

    toolMenu->addAction(zoomOutAction);
    toolMenu->addAction(zoomInAction);

    QToolBar *tb = addToolBar(QStringLiteral("工具列"));
    tb->setMovable(false);
    tb->addAction(openAction);
    tb->addAction(saveAction);
    tb->addSeparator();
    tb->addAction(zoomOutAction);
    tb->addAction(zoomInAction);

    statusBar()->showMessage(QStringLiteral("x: -, y: -, Gray: -"));

    central = new QWidget(this);
    setCentralWidget(central);

    mainLayout = new QHBoxLayout(central);
    leftLayout = new QVBoxLayout();

    mirrorGroup = new QGroupBox(QStringLiteral("鏡射"), central);
    groupLayout = new QVBoxLayout(mirrorGroup);

    hCheckBox = new QCheckBox(QStringLiteral("水平"), mirrorGroup);
    vCheckBox = new QCheckBox(QStringLiteral("垂直"), mirrorGroup);
    mirrorButton = new QPushButton(QStringLiteral("執行"), mirrorGroup);

    groupLayout->addWidget(hCheckBox);
    groupLayout->addWidget(vCheckBox);
    groupLayout->addWidget(mirrorButton);

    leftLayout->addWidget(mirrorGroup);

    rotateDial = new QDial(central);
    rotateDial->setNotchesVisible(true);
    rotateDial->setRange(0, 360);
    rotateDial->setValue(0);
    rotateDial->setFocusPolicy(Qt::NoFocus);

    leftLayout->addWidget(rotateDial);

    vSpacer = new QSpacerItem(20, 58, QSizePolicy::Minimum, QSizePolicy::Expanding);
    leftLayout->addItem(vSpacer);

    mainLayout->addLayout(leftLayout);

    inWin = new ImageLabel(central);
    inWin->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    inWin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    inWin->setStyleSheet("background: white;");

    scrollArea = new QScrollArea(central);
    scrollArea->setWidget(inWin);
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    mainLayout->addWidget(scrollArea);

    srcImg = QImage();
    dstImg = QImage();
    viewImg = QImage();
    clearView();
    updateUiEnabled();

    connect(openAction, SIGNAL(triggered()), this, SLOT(openImage()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveImage()));
    connect(zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));

    connect(mirrorButton, SIGNAL(clicked()), this, SLOT(mirroredImage()));
    connect(rotateDial, SIGNAL(valueChanged(int)), this, SLOT(rotatedImage()));
    connect(inWin, SIGNAL(mouseMoved(QPoint)), this, SLOT(onMouseMoved(QPoint)));
}

ImageTransform::~ImageTransform()
{
}

void ImageTransform::updateUiEnabled()
{
    bool hasImg = !srcImg.isNull() || !dstImg.isNull();

    saveAction->setEnabled(hasImg);
    zoomInAction->setEnabled(hasImg);
    zoomOutAction->setEnabled(hasImg);

    mirrorGroup->setEnabled(hasImg);
    rotateDial->setEnabled(hasImg);
}

void ImageTransform::clearView()
{
    inWin->clear();
    inWin->setFixedSize(600, 400);
    statusBar()->showMessage(QStringLiteral("x: -, y: -, Gray: -"));
}

void ImageTransform::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("開啟圖片"),
        "",
        QStringLiteral("Image Files (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*.*)")
        );

    if (fileName.isEmpty())
        return;

    QImage img;
    if (!img.load(fileName))
        return;

    srcImg = img.convertToFormat(QImage::Format_RGB32);
    dstImg = QImage();
    rotateDial->setValue(0);

    updateView(srcImg);
    updateUiEnabled();
}

void ImageTransform::saveImage()
{
    const QImage &imgToSave = dstImg.isNull() ? srcImg : dstImg;
    if (imgToSave.isNull())
        return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("儲存圖片"),
        "",
        QStringLiteral("PNG Files (*.png);;JPG Files (*.jpg);;BMP Files (*.bmp)")
        );

    if (!fileName.isEmpty())
        imgToSave.save(fileName);
}

void ImageTransform::mirroredImage()
{
    if (srcImg.isNull())
        return;

    const bool H = hCheckBox->isChecked();
    const bool V = vCheckBox->isChecked();

    Qt::Orientations ori;
    if (H) ori |= Qt::Horizontal;
    if (V) ori |= Qt::Vertical;

    if (ori == Qt::Orientations{}) {
        dstImg = srcImg; // 沒勾選就不變
    } else {
        dstImg = srcImg.flipped(ori);
    }

    srcImg = dstImg;

    rotatedImage();
}

void ImageTransform::rotatedImage()
{
    if (srcImg.isNull())
        return;

    const int angle = rotateDial->value();

    QTransform tran;
    tran.rotate(angle);

    dstImg = srcImg.transformed(tran, Qt::SmoothTransformation);
    updateView(dstImg);
}

void ImageTransform::zoomIn()
{
    showZoomWindow(1.25);
}

void ImageTransform::zoomOut()
{
    showZoomWindow(0.80);
}

void ImageTransform::onMouseMoved(const QPoint &pos)
{
    if (viewImg.isNull())
        return;

    const int x = pos.x();
    const int y = pos.y();

    if (x < 0 || y < 0 || x >= viewImg.width() || y >= viewImg.height())
    {
        statusBar()->showMessage(QStringLiteral("x: -, y: -, Gray: -"));
        return;
    }

    const int gray = qGray(viewImg.pixel(x, y));
    statusBar()->showMessage(QStringLiteral("x: %1, y: %2, Gray: %3").arg(x).arg(y).arg(gray));
}

void ImageTransform::updateView(const QImage &img)
{
    if (img.isNull())
    {
        clearView();
        return;
    }

    viewImg = img;
    QPixmap pix = QPixmap::fromImage(img);

    inWin->setPixmap(pix);
    inWin->setFixedSize(pix.size());
}

void ImageTransform::showZoomWindow(double factor)
{
    const QImage &base = dstImg.isNull() ? srcImg : dstImg;
    if (base.isNull())
        return;

    QSize scaledSize(
        qMax(1, static_cast<int>(qRound(base.width()  * factor))),
        qMax(1, static_cast<int>(qRound(base.height() * factor)))
        );

    QPixmap pix = QPixmap::fromImage(base).scaled(
        scaledSize,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
        );

    QDialog *dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    dlg->setWindowTitle(QStringLiteral("Zoom x%1").arg(factor, 0, 'f', 2));

    QVBoxLayout *lay = new QVBoxLayout(dlg);

    QLabel *lab = new QLabel(dlg);
    lab->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    lab->setPixmap(pix);
    lab->setFixedSize(pix.size());

    QScrollArea *sa = new QScrollArea(dlg);
    sa->setWidget(lab);
    sa->setWidgetResizable(false);
    sa->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    lay->addWidget(sa);

    dlg->resize(
        qMin(pix.width() + 40, 900),
        qMin(pix.height() + 80, 700)
        );

    dlg->show();
}
