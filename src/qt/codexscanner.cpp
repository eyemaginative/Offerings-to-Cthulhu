// Copyright (c) 2026 SubGenius.Finance Community
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "codexscanner.h"

#include "main.h"          // chainActive, cs_main, ReadBlockFromDisk
#include "core.h"          // CBlock, CTransaction, CScript

#include <QTimer>

#include <vector>

// Wire format (matches src/miner.cpp::PushEgg):
//   single PUSHDATA carrying:
//     [4]  "OFF1" magic
//     [4]  uint32 LE chunk index
//     [N]  text payload, up to CODEX_CHUNK bytes (48)
// In a coinbase scriptSig the push is preceded by a single-byte length, so the
// pattern we scan for is { L, 'O', 'F', 'F', '1', ... } with L >= 8.

namespace {

static const int      CODEX_ANCHOR_HEIGHT   = 1000001;
static const uint32_t CODEX_MILESTONE_IDX   = 0xFFFFFFFFu;
// Batch size: ~50 blocks per tick is comfortable. A coinbase decode is ~one
// disk read + a tiny scriptSig parse; 50 keeps each tick well under one frame
// at 60 Hz while still letting a stale wallet catch up to a million blocks in
// a few minutes.
static const int      BATCH_SIZE            = 50;

// Try to parse "OFF1 | uint32-LE idx | text" out of a coinbase scriptSig.
// Returns true on success; chunkIdx + text are then filled. Mirrors the
// website pipeline's decoder so on-chain bytes render identically.
static bool DecodeOff1(const CScript &scriptSig, uint32_t &chunkIdx, QString &text)
{
    const std::vector<unsigned char> bytes(scriptSig.begin(), scriptSig.end());
    if (bytes.size() < 9)
        return false;
    // Scan for the magic. The coinbase prefix carries height + extranonce, so
    // OFF1 will not be at offset 0; check up to len-8.
    for (size_t p = 1; p + 8 <= bytes.size(); ++p) {
        if (bytes[p]   != 'O') continue;
        if (bytes[p+1] != 'F') continue;
        if (bytes[p+2] != 'F') continue;
        if (bytes[p+3] != '1') continue;
        // The byte immediately before the magic is the PUSHDATA length.
        unsigned int L = bytes[p-1];
        if (L < 8 || p + L > bytes.size()) continue;
        chunkIdx =  (uint32_t)bytes[p+4]
                 | ((uint32_t)bytes[p+5] <<  8)
                 | ((uint32_t)bytes[p+6] << 16)
                 | ((uint32_t)bytes[p+7] << 24);
        const size_t textLen = L - 8;
        // latin-1 round-trip: the corpus is ASCII (the corpus loader rejects
        // anything else) but the website decoder uses latin-1 to be safe with
        // any future high-bit bytes that might leak in. Match that here.
        QString out;
        out.reserve((int)textLen);
        for (size_t i = 0; i < textLen; ++i)
            out.append(QChar((ushort)bytes[p+8+i]));
        text = out;
        return true;
    }
    return false;
}

} // namespace

CodexScanner::CodexScanner(QObject *parent) :
    QObject(parent),
    timer(0),
    cursor(CODEX_ANCHOR_HEIGHT),
    targetTip(0),
    scanning(false)
{
    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(0);   // re-enter event loop between batches
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
}

CodexScanner::~CodexScanner()
{
}

void CodexScanner::start()
{
    // start() is for the initial nudge — the scan itself begins when scanTo()
    // is told a real tip. If the chain has already reached the Awakening this
    // does nothing surprising; if not, we sit idle until it does.
}

bool CodexScanner::isScanning() const
{
    return scanning;
}

void CodexScanner::scanTo(int tipHeight)
{
    if (tipHeight < CODEX_ANCHOR_HEIGHT)
        return;
    if (tipHeight > targetTip)
        targetTip = tipHeight;
    if (cursor > targetTip) {
        emit caughtUp();
        return;
    }
    if (!scanning) {
        scanning = true;
        timer->start();
    }
}

void CodexScanner::tick()
{
    int processed = 0;
    while (processed < BATCH_SIZE && cursor <= targetTip) {
        CBlock block;
        bool ok = false;
        {
            LOCK(cs_main);
            CBlockIndex *pindex = chainActive[cursor];
            if (pindex)
                ok = ReadBlockFromDisk(block, pindex);
        }
        if (ok && !block.vtx.empty()) {
            const CTransaction &cb = block.vtx[0];
            if (!cb.vin.empty()) {
                uint32_t idx = 0;
                QString  text;
                if (DecodeOff1(cb.vin[0].scriptSig, idx, text)) {
                    CodexFragment frag;
                    frag.height   = cursor;
                    frag.chunkIdx = idx;
                    frag.text     = text;
                    emit fragmentFound(frag);
                } else {
                    emit blockEmpty(cursor);
                }
            }
        }
        ++cursor;
        ++processed;
    }

    emit progress(cursor - 1, targetTip);

    if (cursor > targetTip) {
        timer->stop();
        scanning = false;
        emit caughtUp();
    }
}
