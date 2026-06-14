// Copyright (c) 2026 SubGenius.Finance Community
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef OFFERINGS_QT_CODEXSCANNER_H
#define OFFERINGS_QT_CODEXSCANNER_H

#include <QObject>
#include <QString>
#include <QList>
#include <stdint.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

/** A decoded Codex inscription pulled from a single block's coinbase scriptSig.
 *  text is rendered with latin-1 -> QString (matches the web pipeline's decoder).
 *  Special indices:
 *    0xFFFFFFFF -> CODEX_MILESTONE (the 10 Descent verses, h=999991..1000000)
 *    0xFFFFFFFE -> CODEX_DREAMING  (post-canon R'lyehian, after the corpus is exhausted)
 *  Anything else is a corpus chunk; concatenating chunks 0..N gives the corpus prefix. */
struct CodexFragment
{
    int      height;
    uint32_t chunkIdx;
    QString  text;
};

/** Walks the connected chain looking for "OFF1"-magic inscriptions in coinbase
 *  scriptSigs, decodes them, and emits one signal per fragment found.
 *
 *  The full back-scan can be expensive (a million blocks of disk reads). To keep
 *  the GUI thread responsive we slice it into small batches driven by a QTimer
 *  with a 0-ms interval — Qt re-enters the event loop between batches, so paints
 *  and clicks keep flowing. This sidesteps cross-thread CBlockIndex* lifetime
 *  concerns (we always hold cs_main while touching chainActive / mapBlockIndex). */
class CodexScanner : public QObject
{
    Q_OBJECT
public:
    explicit CodexScanner(QObject *parent = 0);
    ~CodexScanner();

    /** Begin (or resume) the back-scan from the first post-Awakening block.
     *  Idempotent — if a scan is already running this is a no-op. Safe to call
     *  before the chain has reached the Awakening height; it just sits idle. */
    void start();

    /** Scan from cursor up to (and including) tipHeight, one batch per tick.
     *  Called both for the initial back-scan and for incremental top-up after
     *  numBlocksChanged. */
    void scanTo(int tipHeight);

    int  scannedTo() const { return cursor - 1; }
    bool isScanning() const;

signals:
    /** A real OFF1 inscription was decoded out of a coinbase. */
    void fragmentFound(const CodexFragment &frag);

    /** A post-Awakening block had no decodable OFF1 inscription (outsider miner,
     *  pre-Codex daemon, malformed). The reader renders these as gaps. */
    void blockEmpty(int height);

    /** Progress tick — emitted every batch so the UI can show a back-scan bar. */
    void progress(int scannedHeight, int tipHeight);

    /** The cursor has caught up to the tip. */
    void caughtUp();

private slots:
    void tick();

private:
    QTimer *timer;
    int     cursor;     // next height to scan
    int     targetTip;  // highest height we've been asked to scan up to
    bool    scanning;
};

#endif // OFFERINGS_QT_CODEXSCANNER_H
