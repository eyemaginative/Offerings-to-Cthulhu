// Copyright (c) 2026 SubGenius.Finance Community
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "codexpage.h"
#include "clientmodel.h"

#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFont>

CodexPage::CodexPage(QWidget *parent) :
    QWidget(parent),
    clientModel(0),
    titleLabel(0),
    statusLabel(0),
    bodyText(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    titleLabel = new QLabel(tr("The Codex"), this);
    {
        QFont f = titleLabel->font();
        f.setPointSize(f.pointSize() + 8);
        f.setBold(true);
        titleLabel->setFont(f);
    }
    layout->addWidget(titleLabel);

    statusLabel = new QLabel(tr("Awaiting the first inscription."), this);
    statusLabel->setStyleSheet("color: #888;");
    layout->addWidget(statusLabel);

    bodyText = new QTextEdit(this);
    bodyText->setReadOnly(true);
    bodyText->setFrameShape(QFrame::NoFrame);
    bodyText->setHtml(tr(
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
    layout->addWidget(bodyText, 1);
}

CodexPage::~CodexPage()
{
}

void CodexPage::setClientModel(ClientModel *model)
{
    clientModel = model;
    if (!model) return;

    // Hook block-tip updates so the skeleton can prove it's alive.
    // Phase 2 swaps this for the real chain-walker / fragment parser.
    connect(model, SIGNAL(numBlocksChanged(int)),
            this,  SLOT(onNumBlocksChanged(int)));
    onNumBlocksChanged(model->getNumBlocks());
}

void CodexPage::onNumBlocksChanged(int count)
{
    const int forkHeight = 1000000;
    if (count >= forkHeight) {
        statusLabel->setText(tr("Awakened at block %1. Inscription in progress.").arg(count));
    } else {
        int remaining = forkHeight - count;
        statusLabel->setText(tr("%1 blocks until the Awakening (current tip %2).")
                                 .arg(remaining).arg(count));
    }
}
