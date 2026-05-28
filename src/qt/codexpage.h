// Copyright (c) 2026 SubGenius.Finance Community
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CODEXPAGE_H
#define CODEXPAGE_H

#include <QWidget>

class ClientModel;

QT_BEGIN_NAMESPACE
class QLabel;
class QTextEdit;
QT_END_NAMESPACE

/** Codex page — in-wallet reader for the on-chain Lovecraft canon.
 *
 *  Skeleton stage: shows a placeholder and listens for new blocks to
 *  prove the wiring is alive. Phase 2 will add the chain-walker, the
 *  OFF1-fragment parser, and the paginated reader UI.
 */
class CodexPage : public QWidget
{
    Q_OBJECT
public:
    explicit CodexPage(QWidget *parent = 0);
    ~CodexPage();

    void setClientModel(ClientModel *clientModel);

private:
    ClientModel *clientModel;
    QLabel      *titleLabel;
    QLabel      *statusLabel;
    QTextEdit   *bodyText;

private slots:
    void onNumBlocksChanged(int count);
};

#endif // CODEXPAGE_H
