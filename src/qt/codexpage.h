// Copyright (c) 2026 SubGenius.Finance Community
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CODEXPAGE_H
#define CODEXPAGE_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QVector>
#include <QList>

#include <stdint.h>

class ClientModel;
class CodexScanner;
struct CodexFragment;

QT_BEGIN_NAMESPACE
class QLabel;
class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QPushButton;
class QStackedWidget;
class QSplitter;
QT_END_NAMESPACE

/** Codex page — in-wallet reader for the on-chain Lovecraft canon.
 *
 *  Three-pane layout:
 *    - Left: Table of Contents, one row per book in the manifest, with a
 *      per-book completion bar driven by inscribed bytes.
 *    - Center: paginated reader showing the currently-selected book. Each
 *      paragraph is reconstructed from chunks inscribed in the chain; the
 *      page footer cites the range of blocks that wrote it.
 *    - Right (bottom on narrow displays): Frontier panel, a tail of the
 *      latest N inscriptions as they arrive.
 *
 *  Until the Awakening height (block 1,000,000) the page renders the
 *  Phase-A placeholder; after that the three-pane reader engages and a
 *  CodexScanner walks the chain to populate it. */
class CodexPage : public QWidget
{
    Q_OBJECT
public:
    explicit CodexPage(QWidget *parent = 0);
    ~CodexPage();

    void setClientModel(ClientModel *clientModel);

private slots:
    void onNumBlocksChanged(int count);
    void onFragmentFound(const CodexFragment &frag);
    void onScannerProgress(int scanned, int target);
    void onBookSelected(int row);
    void onPrevPage();
    void onNextPage();

private:
    enum Mode { ModePlaceholder, ModeReader };

    struct Fragment {
        int     height;     // block height
        QString text;       // payload as latin-1 -> QString
        Fragment() : height(0) {}
        Fragment(int h, const QString &t) : height(h), text(t) {}
    };

    struct Book {
        QString  title;
        int      bodyStart;     // byte offset in the corpus (per manifest)
        int      bodyLen;       // length in bytes
        int      bytesInscribed;// running count, clamped to bodyLen
    };

    struct Page {
        QString html;
        int     firstHeight;    // block that inscribed this page's first byte (-1 if unknown)
        int     lastHeight;     // block that inscribed this page's last byte  (-1 if unknown)
    };

    void buildUi();
    void switchMode(Mode mode);
    void hookScanner();
    void seedManifest();
    void rebuildTocRow(int bookIdx);
    void rebuildSelectedBookPages();
    void renderCurrentPage();
    void updateHeaderStatus();
    void appendFrontier(int height, uint32_t chunkIdx, const QString &text);
    int  totalCorpusBytes() const;

    ClientModel  *clientModel;
    CodexScanner *scanner;
    Mode          mode;

    // Placeholder mode widgets (Phase A — pre-Awakening).
    QWidget      *placeholderPane;
    QLabel       *titleLabel;
    QLabel       *statusLabel;
    QTextEdit    *placeholderBody;

    // Reader mode widgets.
    QWidget      *readerPane;
    QLabel       *headerStatus;
    QListWidget  *tocList;
    QTextEdit    *readerView;
    QLabel       *pageFooter;
    QPushButton  *prevButton;
    QPushButton  *nextButton;
    QLabel       *pageCounter;
    QListWidget  *frontierList;

    QStackedWidget *stack;
    QSplitter      *splitter;

    // State.
    QVector<Book>                books;
    QMap<uint32_t, Fragment>     chunks;        // chunk_idx -> (height, text)
    QMap<int, Fragment>          descent;       // height -> verse (h in 999991..1000000)
    QVector<Fragment>            dreaming;      // post-canon R'lyehian, in arrival order
    QVector<Page>                currentPages;
    int                          selectedBook;
    int                          currentPage;
    int                          chainHeight;
    int                          inscribedContiguous; // bytes from chunk 0 up to first gap
};

#endif // CODEXPAGE_H
