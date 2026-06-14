// Copyright (c) 2026 SubGenius.Finance Community
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "codexpage.h"
#include "clientmodel.h"
#include "codexscanner.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm>

// ---- Constants --------------------------------------------------------------

namespace {

// Mirrors src/miner.cpp: canon transcription begins right after the Awakening,
// 48 bytes per coinbase, descent verses occupy heights 999991..1000000.
static const int      CODEX_FORK_HEIGHT     = 1000000;
static const int      CODEX_ANCHOR_HEIGHT   = 1000001;
static const int      CODEX_DESCENT_START   = 999991;
static const int      CODEX_CHUNK           = 48;
static const uint32_t CODEX_MILESTONE_IDX   = 0xFFFFFFFFu;
static const uint32_t CODEX_DREAMING_IDX    = 0xFFFFFFFEu;

// Pagination policy: match the website e-reader's ~2,600-char pages so the
// in-wallet reader looks the same as 23skidoo.info/codex/.
static const int      PAGE_CHAR_TARGET      = 2600;

// Frontier ring buffer size. Anything older than this scrolls off; the goal
// is "what just got written," not history.
static const int      FRONTIER_KEEP         = 64;

// ---- Manifest (baked in from contrib/codex/codex_manifest.json) ------------
// We don't parse codex_corpus.txt at runtime — per the design constraint, the
// reader displays only on-chain bytes. The manifest's *structural* metadata
// (book titles, body offsets, body lengths) is the only thing baked in: it's
// what tells us where each book begins and ends in the corpus byte stream,
// which we need to slot inscribed chunks into the right book in the TOC.

struct BookManifestEntry {
    const char *title;
    int         bodyStart;
    int         bodyLen;
};

static const BookManifestEntry kBooks[] = {
    { "Proem \xE2\x80\x94 The Conclave's Invocation",          37,    453 },
    { "The call of Cthulhu",                                  511,  71203 },
    { "The silver key",                                     71730,  28681 },
    { "The Dunwich horror",                                100431, 101607 },
    { "The colour out of space",                           202063,  69997 },
    { "He",                                                272064,  24492 },
    { "The festival",                                      296570,  20750 },
    { "The lurking fear",                                  317338,  48582 },
    { "The last test",                                     365935, 112169 },
    { "At the mountains of madness",                       478133, 245791 },
    { "Medusa's coil",                                     723939,  90140 },
    { "The curse of Yig",                                  814097,  39770 },
    { "Through the gates of the silver key",               853904,  85551 },
    { "The horror at Red Hook",                            939479,  50263 },
    { "Cool air",                                          989752,  20450 },
    { "The shadow over Innsmouth",                        1010229,  96913 },
    { "The quest of Iranon",                              1107163,  14997 },
    { "The thing on the door-step",                       1122188,  61758 },
    { "The haunter of the dark",                          1183971,  55638 },
    { "The trap",                                         1239619,  51366 },
    { "The case of Charles Dexter Ward",                  1291018, 264228 },
    { "The horror in the burying-ground",                 1555280,  33526 },
    { "The Shunned House",                                1588825,  64044 },
    { "Writings in the United Amateur, 1915-1922",        1652912, 614955 },
};
static const int kNumBooks = (int)(sizeof(kBooks) / sizeof(kBooks[0]));
static const int kCorpusBytes = 2267867;   // matches manifest "total"

// HTML-escape a QString for embedding in QTextBrowser HTML.
static QString escape(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
        QChar c = s.at(i);
        switch (c.unicode()) {
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '&': out += "&amp;"; break;
            case '"': out += "&quot;"; break;
            default:  out += c;
        }
    }
    return out;
}

// Group a flat byte stream into paragraphs the way the corpus is structured:
// blank line -> paragraph break, single newlines collapsed to spaces. Matches
// build_ereader.py::reflow_paragraphs.
static QStringList reflowParagraphs(const QString &raw)
{
    QStringList paras;
    QString current;
    bool blankSeen = false;
    for (int i = 0; i < raw.size(); ++i) {
        QChar c = raw.at(i);
        if (c == QChar('\n')) {
            // peek ahead: blank line = paragraph break
            int j = i + 1;
            while (j < raw.size() && (raw.at(j) == QChar(' ') || raw.at(j) == QChar('\t')))
                ++j;
            if (j < raw.size() && raw.at(j) == QChar('\n')) {
                if (!current.trimmed().isEmpty())
                    paras.append(current.trimmed());
                current.clear();
                blankSeen = true;
                i = j; // consume the second newline; loop ++i moves past it
                continue;
            }
            current += QChar(' ');
        } else {
            current += c;
            blankSeen = false;
        }
    }
    if (!current.trimmed().isEmpty())
        paras.append(current.trimmed());
    (void)blankSeen;
    return paras;
}

} // namespace

// ---- ctor / dtor / UI --------------------------------------------------------

CodexPage::CodexPage(QWidget *parent) :
    QWidget(parent),
    clientModel(0),
    scanner(0),
    mode(ModePlaceholder),
    placeholderPane(0),
    titleLabel(0),
    statusLabel(0),
    placeholderBody(0),
    readerPane(0),
    headerStatus(0),
    tocList(0),
    readerView(0),
    pageFooter(0),
    prevButton(0),
    nextButton(0),
    pageCounter(0),
    frontierList(0),
    stack(0),
    splitter(0),
    selectedBook(0),
    currentPage(0),
    chainHeight(0),
    inscribedContiguous(0)
{
    seedManifest();
    buildUi();
    switchMode(ModePlaceholder);
}

CodexPage::~CodexPage()
{
}

void CodexPage::seedManifest()
{
    books.clear();
    books.reserve(kNumBooks);
    for (int i = 0; i < kNumBooks; ++i) {
        Book b;
        b.title          = QString::fromUtf8(kBooks[i].title);
        b.bodyStart      = kBooks[i].bodyStart;
        b.bodyLen        = kBooks[i].bodyLen;
        b.bytesInscribed = 0;
        books.append(b);
    }
}

void CodexPage::buildUi()
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(8);

    stack = new QStackedWidget(this);
    outer->addWidget(stack, 1);

    // -------- Placeholder pane (pre-Awakening) -------------------------------
    placeholderPane = new QWidget(this);
    QVBoxLayout *pv = new QVBoxLayout(placeholderPane);
    pv->setContentsMargins(24, 24, 24, 24);
    pv->setSpacing(12);

    titleLabel = new QLabel(tr("The Codex"), placeholderPane);
    {
        QFont f = titleLabel->font();
        f.setPointSize(f.pointSize() + 8);
        f.setBold(true);
        titleLabel->setFont(f);
    }
    pv->addWidget(titleLabel);

    statusLabel = new QLabel(tr("Awaiting the first inscription."), placeholderPane);
    statusLabel->setStyleSheet("color: #888;");
    pv->addWidget(statusLabel);

    placeholderBody = new QTextEdit(placeholderPane);
    placeholderBody->setReadOnly(true);
    placeholderBody->setFrameShape(QFrame::NoFrame);
    placeholderBody->setHtml(tr(
        "<p>The chain begins transcribing the public-domain Lovecraft canon at "
        "block <b>1,000,001</b>, one fragment per coinbase. Until activation, "
        "this page stands as a placeholder.</p>"
        "<p>When the inscription begins, this page will show:</p>"
        "<ul>"
        "<li>A Table of Contents for the 23 books, split on the per-work sentinel byte</li>"
        "<li>A paginated reader with per-block citations</li>"
        "<li>A Frontier view tailing the sentence currently being written</li>"
        "</ul>"
        "<p style='color:#888;font-style:italic;'>"
        "Awaiting the canon."
        "</p>"
    ));
    pv->addWidget(placeholderBody, 1);
    stack->addWidget(placeholderPane);

    // -------- Reader pane (post-Awakening) -----------------------------------
    readerPane = new QWidget(this);
    QVBoxLayout *rv = new QVBoxLayout(readerPane);
    rv->setContentsMargins(0, 0, 0, 0);
    rv->setSpacing(6);

    headerStatus = new QLabel(readerPane);
    headerStatus->setTextFormat(Qt::RichText);
    headerStatus->setStyleSheet("color: #cfeee0; padding: 4px 6px;");
    rv->addWidget(headerStatus);

    splitter = new QSplitter(Qt::Horizontal, readerPane);
    splitter->setChildrenCollapsible(false);

    // -- TOC (left) --
    QWidget *leftWrap = new QWidget(splitter);
    QVBoxLayout *lv = new QVBoxLayout(leftWrap);
    lv->setContentsMargins(0, 0, 0, 0);
    lv->setSpacing(4);
    QLabel *tocLabel = new QLabel(tr("The Library"), leftWrap);
    {
        QFont f = tocLabel->font(); f.setBold(true);
        tocLabel->setFont(f);
    }
    lv->addWidget(tocLabel);
    tocList = new QListWidget(leftWrap);
    tocList->setUniformItemSizes(false);
    tocList->setSelectionMode(QAbstractItemView::SingleSelection);
    lv->addWidget(tocList, 1);
    splitter->addWidget(leftWrap);

    // -- Reader (center) --
    QWidget *centerWrap = new QWidget(splitter);
    QVBoxLayout *cv = new QVBoxLayout(centerWrap);
    cv->setContentsMargins(0, 0, 0, 0);
    cv->setSpacing(4);
    readerView = new QTextEdit(centerWrap);
    readerView->setReadOnly(true);
    cv->addWidget(readerView, 1);

    QHBoxLayout *navRow = new QHBoxLayout();
    prevButton  = new QPushButton(tr("\xE2\x86\x90 prev"), centerWrap);
    nextButton  = new QPushButton(tr("next \xE2\x86\x92"), centerWrap);
    pageCounter = new QLabel(tr("page \xE2\x80\x94"), centerWrap);
    pageCounter->setAlignment(Qt::AlignCenter);
    navRow->addWidget(prevButton);
    navRow->addStretch();
    navRow->addWidget(pageCounter);
    navRow->addStretch();
    navRow->addWidget(nextButton);
    cv->addLayout(navRow);

    pageFooter = new QLabel(centerWrap);
    pageFooter->setStyleSheet("color: #5fffd0; font-family: monospace; font-size: 11px;");
    pageFooter->setAlignment(Qt::AlignCenter);
    cv->addWidget(pageFooter);
    splitter->addWidget(centerWrap);

    // -- Frontier (right) --
    QWidget *rightWrap = new QWidget(splitter);
    QVBoxLayout *rrv = new QVBoxLayout(rightWrap);
    rrv->setContentsMargins(0, 0, 0, 0);
    rrv->setSpacing(4);
    QLabel *frontierLabel = new QLabel(tr("Frontier"), rightWrap);
    {
        QFont f = frontierLabel->font(); f.setBold(true);
        frontierLabel->setFont(f);
    }
    rrv->addWidget(frontierLabel);
    frontierList = new QListWidget(rightWrap);
    frontierList->setUniformItemSizes(false);
    frontierList->setAlternatingRowColors(true);
    frontierList->setSelectionMode(QAbstractItemView::NoSelection);
    rrv->addWidget(frontierList, 1);
    splitter->addWidget(rightWrap);

    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 5);
    splitter->setStretchFactor(2, 2);
    QList<int> sizes;
    sizes << 220 << 560 << 220;
    splitter->setSizes(sizes);

    rv->addWidget(splitter, 1);
    stack->addWidget(readerPane);

    connect(tocList,    SIGNAL(currentRowChanged(int)), this, SLOT(onBookSelected(int)));
    connect(prevButton, SIGNAL(clicked()),              this, SLOT(onPrevPage()));
    connect(nextButton, SIGNAL(clicked()),              this, SLOT(onNextPage()));

    // Seed the TOC with rows now so it's not empty when the user first visits.
    for (int i = 0; i < books.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(tocList);
        item->setData(Qt::UserRole, i);
        tocList->addItem(item);
        rebuildTocRow(i);
    }
    if (!books.isEmpty())
        tocList->setCurrentRow(0);

    updateHeaderStatus();
}

void CodexPage::switchMode(Mode m)
{
    mode = m;
    if (!stack) return;
    stack->setCurrentWidget(m == ModeReader ? readerPane : placeholderPane);
}

// ---- ClientModel wiring ------------------------------------------------------

void CodexPage::setClientModel(ClientModel *model)
{
    clientModel = model;
    if (!model) return;

    if (!scanner) {
        scanner = new CodexScanner(this);
        connect(scanner, SIGNAL(fragmentFound(const CodexFragment&)),
                this,    SLOT(onFragmentFound(const CodexFragment&)));
        connect(scanner, SIGNAL(progress(int,int)),
                this,    SLOT(onScannerProgress(int,int)));
    }

    connect(model, SIGNAL(numBlocksChanged(int)),
            this,  SLOT(onNumBlocksChanged(int)));
    onNumBlocksChanged(model->getNumBlocks());
}

void CodexPage::onNumBlocksChanged(int count)
{
    chainHeight = count;
    if (count >= CODEX_FORK_HEIGHT) {
        if (mode != ModeReader)
            switchMode(ModeReader);
        if (scanner)
            scanner->scanTo(count);
    } else {
        int remaining = CODEX_FORK_HEIGHT - count;
        statusLabel->setText(tr("%1 blocks until the Awakening (current tip %2).")
                                 .arg(remaining).arg(count));
        switchMode(ModePlaceholder);
    }
    updateHeaderStatus();
}

// ---- Scanner callbacks -------------------------------------------------------

void CodexPage::onFragmentFound(const CodexFragment &frag)
{
    if (frag.chunkIdx == CODEX_MILESTONE_IDX) {
        // Descent verse (one of the 10 ceremonial blocks 999991..1000000).
        descent.insert(frag.height, Fragment(frag.height, frag.text));
        appendFrontier(frag.height, frag.chunkIdx, frag.text);
        // Descent verses don't live in the book TOC — they get their own
        // section in the reader's status line. No book rebuild needed.
        updateHeaderStatus();
        return;
    }

    if (frag.chunkIdx == CODEX_DREAMING_IDX) {
        // Phase B: the canon is exhausted; the chain speaks R'lyehian. Stored
        // in arrival order; rendered as Book 24+1 conceptually but here we just
        // surface them on the Frontier with a marker.
        dreaming.append(Fragment(frag.height, frag.text));
        appendFrontier(frag.height, frag.chunkIdx, frag.text);
        updateHeaderStatus();
        return;
    }

    // Regular corpus chunk.
    chunks.insert(frag.chunkIdx, Fragment(frag.height, frag.text));
    appendFrontier(frag.height, frag.chunkIdx, frag.text);

    // Recompute contiguous prefix (bytes from chunk 0 with no gap).
    while (chunks.contains((uint32_t)(inscribedContiguous / CODEX_CHUNK))) {
        inscribedContiguous += CODEX_CHUNK;
    }
    if (inscribedContiguous > kCorpusBytes)
        inscribedContiguous = kCorpusBytes;

    // Update per-book inscription counter. A chunk spans bytes
    // [idx*CHUNK, (idx+1)*CHUNK) and may straddle a book boundary; credit each
    // book the overlap it touches.
    const int chunkStart = (int)frag.chunkIdx * CODEX_CHUNK;
    const int chunkEnd   = chunkStart + frag.text.size();
    for (int b = 0; b < books.size(); ++b) {
        const int bs = books[b].bodyStart;
        const int be = bs + books[b].bodyLen;
        const int lo = std::max(chunkStart, bs);
        const int hi = std::min(chunkEnd, be);
        if (hi > lo) {
            books[b].bytesInscribed = std::min(books[b].bodyLen,
                                               books[b].bytesInscribed + (hi - lo));
            rebuildTocRow(b);
            if (b == selectedBook)
                rebuildSelectedBookPages();
        }
    }

    updateHeaderStatus();
}

void CodexPage::onScannerProgress(int /*scanned*/, int /*target*/)
{
    updateHeaderStatus();
}

// ---- TOC / reader rendering --------------------------------------------------

void CodexPage::rebuildTocRow(int bookIdx)
{
    if (bookIdx < 0 || bookIdx >= books.size()) return;
    if (!tocList || bookIdx >= tocList->count()) return;
    const Book &b = books[bookIdx];
    int pct = b.bodyLen > 0 ? (int)(100LL * b.bytesInscribed / b.bodyLen) : 0;
    if (pct > 100) pct = 100;
    QString label = QString("%1.  %2   [%3%]")
        .arg(bookIdx + 1, 2)
        .arg(b.title)
        .arg(pct, 3);
    QListWidgetItem *item = tocList->item(bookIdx);
    if (item)
        item->setText(label);
}

void CodexPage::onBookSelected(int row)
{
    if (row < 0 || row >= books.size())
        return;
    selectedBook = row;
    currentPage  = 0;
    rebuildSelectedBookPages();
}

void CodexPage::rebuildSelectedBookPages()
{
    currentPages.clear();
    if (selectedBook < 0 || selectedBook >= books.size()) {
        renderCurrentPage();
        return;
    }
    const Book &b = books[selectedBook];

    // Assemble the book's byte range from inscribed chunks. For any chunk
    // that has not yet been inscribed, splice in a placeholder so the reader
    // is honest about where the chain has caught up to.
    const int chunkFirst = b.bodyStart / CODEX_CHUNK;
    const int chunkLast  = (b.bodyStart + b.bodyLen - 1) / CODEX_CHUNK;

    QString assembled;
    QList<int> heightsByChar;   // 1:1 with assembled chars: which block wrote each char
    assembled.reserve(b.bodyLen + 64);

    for (int ci = chunkFirst; ci <= chunkLast; ++ci) {
        const int chunkByteStart = ci * CODEX_CHUNK;
        const int chunkByteEnd   = chunkByteStart + CODEX_CHUNK;
        const int lo = std::max(chunkByteStart, b.bodyStart);
        const int hi = std::min(chunkByteEnd,  b.bodyStart + b.bodyLen);
        const int sliceLen = hi - lo;
        if (sliceLen <= 0) continue;

        QMap<uint32_t, Fragment>::const_iterator it = chunks.find((uint32_t)ci);
        if (it == chunks.constEnd()) {
            // Unknown chunk — mark with a small visible gap.
            const QString gap = QString(" \xE2\x80\xA6 ");  // " … "
            assembled += gap;
            for (int k = 0; k < gap.size(); ++k) heightsByChar.append(-1);
            continue;
        }
        const Fragment &f = it.value();
        // The fragment may be shorter than CODEX_CHUNK (final chunk of corpus).
        const int offset = lo - chunkByteStart;
        const int take   = std::min(sliceLen, f.text.size() - offset);
        if (take <= 0) continue;
        const QString slice = f.text.mid(offset, take);
        assembled += slice;
        for (int k = 0; k < slice.size(); ++k) heightsByChar.append(f.height);
    }

    QStringList paras = reflowParagraphs(assembled);
    if (paras.isEmpty()) {
        Page empty;
        empty.html = QString("<p style='color:#888;font-style:italic;'>")
                   + tr("This work has not yet been inscribed. Return when the Old One speaks.")
                   + "</p>";
        empty.firstHeight = -1;
        empty.lastHeight  = -1;
        currentPages.append(empty);
        renderCurrentPage();
        return;
    }

    // Build pages by accumulating paragraphs until we cross PAGE_CHAR_TARGET.
    // For each page, citation = min/max heightsByChar across its char span.
    // The char->height map drifted during paragraph reflow (newlines became
    // spaces and we trimmed), so for a precise citation we'd need to thread
    // the offset through. Good enough: track running byte cursor through the
    // assembled string and pair each page's start/end character positions
    // with assembled-space heights at those positions.
    int assembledCursor = 0;
    int pageStartCursor = 0;
    QString pageHtml;
    int pageLen = 0;
    for (int p = 0; p < paras.size(); ++p) {
        const QString &para = paras.at(p);
        if (pageLen + para.size() > PAGE_CHAR_TARGET && pageLen > 0) {
            // Flush current page.
            Page page;
            page.html = pageHtml;
            page.firstHeight = -1;
            page.lastHeight  = -1;
            for (int q = pageStartCursor; q < std::min(assembledCursor, heightsByChar.size()); ++q) {
                int h = heightsByChar.at(q);
                if (h < 0) continue;
                if (page.firstHeight < 0 || h < page.firstHeight) page.firstHeight = h;
                if (h > page.lastHeight) page.lastHeight = h;
            }
            currentPages.append(page);
            pageStartCursor = assembledCursor;
            pageHtml.clear();
            pageLen = 0;
        }
        pageHtml += QString("<p style='text-indent:1.4em;text-align:justify;margin:0 0 1em 0;'>")
                  + escape(para)
                  + "</p>";
        pageLen += para.size();
        assembledCursor += para.size();
        // skip past the paragraph-break whitespace in the assembled stream
        // (approximate; for citation purposes this is good enough)
        if (assembledCursor < assembled.size() && assembled.at(assembledCursor) == QChar('\n'))
            ++assembledCursor;
    }
    if (!pageHtml.isEmpty()) {
        Page page;
        page.html = pageHtml;
        page.firstHeight = -1;
        page.lastHeight  = -1;
        for (int q = pageStartCursor; q < std::min(assembledCursor, heightsByChar.size()); ++q) {
            int h = heightsByChar.at(q);
            if (h < 0) continue;
            if (page.firstHeight < 0 || h < page.firstHeight) page.firstHeight = h;
            if (h > page.lastHeight) page.lastHeight = h;
        }
        currentPages.append(page);
    }

    if (currentPage >= currentPages.size())
        currentPage = currentPages.size() - 1;
    if (currentPage < 0)
        currentPage = 0;

    renderCurrentPage();
}

void CodexPage::renderCurrentPage()
{
    if (!readerView) return;
    if (currentPages.isEmpty()) {
        readerView->setHtml(QString("<p style='color:#888;'>")
                            + tr("Awaiting inscription.") + "</p>");
        pageCounter->setText(tr("page \xE2\x80\x94"));
        pageFooter->setText(QString());
        prevButton->setEnabled(false);
        nextButton->setEnabled(false);
        return;
    }
    const Page &p = currentPages.at(currentPage);
    readerView->setHtml(
        QString("<div style='font-family:Georgia,serif;font-size:14px;color:#cfeee0;'>")
        + p.html + "</div>");

    pageCounter->setText(tr("page %1 / %2").arg(currentPage + 1).arg(currentPages.size()));

    if (p.firstHeight > 0 && p.lastHeight > 0) {
        if (p.firstHeight == p.lastHeight) {
            pageFooter->setText(tr("inscribed at block %1").arg(p.firstHeight));
        } else {
            pageFooter->setText(tr("inscribed in blocks %1 \xE2\x80\x93 %2")
                                .arg(p.firstHeight).arg(p.lastHeight));
        }
    } else {
        pageFooter->setText(tr("not yet inscribed on-chain"));
    }

    prevButton->setEnabled(currentPage > 0);
    nextButton->setEnabled(currentPage < currentPages.size() - 1);
}

void CodexPage::onPrevPage()
{
    if (currentPage > 0) {
        --currentPage;
        renderCurrentPage();
    }
}

void CodexPage::onNextPage()
{
    if (currentPage < currentPages.size() - 1) {
        ++currentPage;
        renderCurrentPage();
    }
}

// ---- Frontier + header status ------------------------------------------------

void CodexPage::appendFrontier(int height, uint32_t chunkIdx, const QString &text)
{
    if (!frontierList) return;
    QString prefix;
    if (chunkIdx == CODEX_MILESTONE_IDX) {
        prefix = QString("blk %1  [Descent]   ").arg(height);
    } else if (chunkIdx == CODEX_DREAMING_IDX) {
        prefix = QString("blk %1  [Dreaming]  ").arg(height);
    } else {
        prefix = QString("blk %1  #%2  ").arg(height).arg(chunkIdx);
    }
    QString preview = text;
    preview.replace(QChar('\n'), QChar(' '));
    if (preview.size() > 60)
        preview = preview.left(60) + QString("\xE2\x80\xA6");
    QListWidgetItem *item = new QListWidgetItem(prefix + preview);
    frontierList->insertItem(0, item);
    while (frontierList->count() > FRONTIER_KEEP) {
        QListWidgetItem *gone = frontierList->takeItem(frontierList->count() - 1);
        delete gone;
    }
}

int CodexPage::totalCorpusBytes() const
{
    return kCorpusBytes;
}

void CodexPage::updateHeaderStatus()
{
    if (!headerStatus) return;
    const int totalChunks      = (kCorpusBytes + CODEX_CHUNK - 1) / CODEX_CHUNK;
    const int inscribedChunks  = chunks.size();
    int fullyReadable = 0;
    for (int i = 0; i < books.size(); ++i)
        if (books[i].bytesInscribed >= books[i].bodyLen) ++fullyReadable;
    const int pct = totalChunks > 0
        ? (int)(100LL * inscribedContiguous / kCorpusBytes)
        : 0;
    QString scanLine;
    if (scanner && scanner->isScanning() && chainHeight > 0) {
        int sc = scanner->scannedTo();
        if (sc < CODEX_ANCHOR_HEIGHT) sc = CODEX_ANCHOR_HEIGHT - 1;
        scanLine = QString(" \xC2\xB7 scanning %1 / %2").arg(sc).arg(chainHeight);
    }
    QString descentLine;
    if (!descent.isEmpty()) {
        descentLine = QString(" \xC2\xB7 %1 / 10 Descent verses").arg(descent.size());
    }
    QString dreamingLine;
    if (!dreaming.isEmpty()) {
        dreamingLine = QString(" \xC2\xB7 %1 Dreaming verses").arg(dreaming.size());
    }
    headerStatus->setText(
        QString("<b>The Codex</b> \xE2\x80\x94 height <b>%1</b>"
                " \xC2\xB7 <b>%2</b> / %3 fragments"
                " \xC2\xB7 <b>%4</b> / %5 books complete"
                " \xC2\xB7 <b>%6%</b> of canon contiguous%7%8%9")
        .arg(chainHeight)
        .arg(inscribedChunks).arg(totalChunks)
        .arg(fullyReadable).arg(books.size())
        .arg(pct)
        .arg(scanLine)
        .arg(descentLine)
        .arg(dreamingLine));
}
