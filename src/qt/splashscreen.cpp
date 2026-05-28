// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "splashscreen.h"
#include <boost/version.hpp>
#include <boost/bind/placeholders.hpp>
#if BOOST_VERSION >= 106000
using namespace boost::placeholders;
#endif


#include "clientversion.h"
#include "init.h"
#include "ui_interface.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif

#include <QApplication>
#include <QPainter>

SplashScreen::SplashScreen(const QPixmap &pixmap, Qt::WindowFlags f, bool isTestNet) :
    QSplashScreen(pixmap, f)
{
    setAutoFillBackground(true);

    // text to render — split across top-left (brand), bottom-left (Awakening
    // verse), bottom-right (copyright stack). The splash PNG carries the art
    // and the silver bezel; this code overlays the typography.
    QString titleText       = tr("Cthulhu Offerings");
    QString versionText     = QString(tr("Version %1 \xE2\x80\x94 Restoration"))
                                .arg(QString::fromStdString(FormatFullVersion()));
    // R'lyehian chant — Descent verse pinned at block 1,000,000 in chainparams.
    QString chantText       = QString::fromUtf8("ph\xE2\x80\x99nglui mglw\xE2\x80\x99nafh Cthulhu R\xE2\x80\x99lyeh wgah\xE2\x80\x99nagl fhtagn");
    QString awakensText     = tr("He Awakens.");
    QString blockText       = tr("at Block 1,000,000");
    QString copyDevsText    = QChar(0xA9) + QString(" %1 ").arg(COPYRIGHT_YEAR)
                                + tr("The Offerings to Cthulhu Developers");
    QString copyCommText    = QChar(0xA9) + QString(" %1 ").arg(COPYRIGHT_YEAR)
                                + tr("by SubGenius.Finance Community");
    QString taglineText     = tr("SubGenius.Finance: Where Sub-Culture Becomes Capital");
    QString testnetAddText  = tr("[TESTNET]");

    QString serif = "Serif";  // Qt resolves to system default serif (DejaVu/Liberation/Times)

    // load the bitmap for writing some text over it
    QPixmap newPixmap;
    if(isTestNet) {
        newPixmap     = QPixmap(":/images/splash_testnet");
    }
    else {
        newPixmap     = QPixmap(":/images/splash");
    }

    QPainter pixPaint(&newPixmap);
    pixPaint.setRenderHint(QPainter::Antialiasing);
    pixPaint.setRenderHint(QPainter::TextAntialiasing);

    int w = newPixmap.width();
    int h = newPixmap.height();

    // === top-left: project title + version ===
    pixPaint.setPen(QColor(0xdd, 0xa1, 0x49));  // gold accent
    pixPaint.setFont(QFont(serif, 18, QFont::Bold));
    pixPaint.drawText(20, 32, titleText);

    pixPaint.setPen(QColor(0xcc, 0xcc, 0xcc));
    pixPaint.setFont(QFont(serif, 9));
    pixPaint.drawText(20, 52, versionText);

    // === bottom-left: chant → rupture → block ===
    pixPaint.setPen(QColor(0xd4, 0xc7, 0x9a));  // muted gold
    QFont chantFont(serif, 11);
    chantFont.setItalic(true);
    pixPaint.setFont(chantFont);
    pixPaint.drawText(22, h - 130, chantText);

    pixPaint.setPen(QColor(0xdd, 0xa1, 0x49));
    pixPaint.setFont(QFont(serif, 32, QFont::Bold));
    pixPaint.drawText(22, h - 75, awakensText);

    pixPaint.setPen(QColor(0xa8, 0xa8, 0xa8));
    QFont blockFont(serif, 11);
    blockFont.setItalic(true);
    pixPaint.setFont(blockFont);
    pixPaint.drawText(24, h - 50, blockText);

    // === bottom-right: copyright stack + SubGenius.Finance tagline ===
    pixPaint.setPen(QColor(0xb8, 0xb8, 0xb8));
    pixPaint.setFont(QFont(serif, 9));
    QFontMetrics fmCopy = pixPaint.fontMetrics();
    pixPaint.drawText(w - fmCopy.width(copyDevsText) - 18, h - 58, copyDevsText);
    pixPaint.drawText(w - fmCopy.width(copyCommText) - 18, h - 42, copyCommText);

    pixPaint.setPen(QColor(0xdd, 0xa1, 0x49));
    pixPaint.setFont(QFont(serif, 9, QFont::Bold));
    QFontMetrics fmTag = pixPaint.fontMetrics();
    pixPaint.drawText(w - fmTag.width(taglineText) - 18, h - 18, taglineText);

    // testnet stamp — bold red across upper-center
    if(isTestNet) {
        QFont testnetFont(serif, 16, QFont::Bold);
        pixPaint.setFont(testnetFont);
        pixPaint.setPen(QColor(0xc0, 0x40, 0x40));
        QFontMetrics fmTn = pixPaint.fontMetrics();
        pixPaint.drawText((w - fmTn.width(testnetAddText)) / 2, 90, testnetAddText);
    }

    pixPaint.end();

    this->setPixmap(newPixmap);

    subscribeToCoreSignals();
}

SplashScreen::~SplashScreen()
{
    unsubscribeFromCoreSignals();
}

void SplashScreen::slotFinish(QWidget *mainWin)
{
    finish(mainWin);
}

static void InitMessage(SplashScreen *splash, const std::string &message)
{
    QMetaObject::invokeMethod(splash, "showMessage",
        Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(message)),
        Q_ARG(int, Qt::AlignBottom|Qt::AlignHCenter),
        Q_ARG(QColor, QColor(55,55,55)));
}

static void ShowProgress(SplashScreen *splash, const std::string &title, int nProgress)
{
    InitMessage(splash, title + strprintf("%d", nProgress) + "%");
}

#ifdef ENABLE_WALLET
static void ConnectWallet(SplashScreen *splash, CWallet* wallet)
{
    wallet->ShowProgress.connect(boost::bind(ShowProgress, splash, _1, _2));
}
#endif

void SplashScreen::subscribeToCoreSignals()
{
    // Connect signals to client
    uiInterface.InitMessage.connect(boost::bind(InitMessage, this, _1));
#ifdef ENABLE_WALLET
    uiInterface.LoadWallet.connect(boost::bind(ConnectWallet, this, _1));
#endif
}

void SplashScreen::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    uiInterface.InitMessage.disconnect(boost::bind(InitMessage, this, _1));
#ifdef ENABLE_WALLET
    if(pwalletMain)
        pwalletMain->ShowProgress.disconnect(boost::bind(ShowProgress, this, _1, _2));
#endif
}
