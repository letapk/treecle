/* Treecle
 *
 * Copyright (c) Kartik Patel
 * E-mail: letapk@gmail.com
 * Download from: https://letapk.wordpress.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

//Last modified Sept 15, 2026

#include "treecle.h"

#include <QTextCursor>
#include <QTextCharFormat>

#include <utility>

void collapse (QTreeWidgetItem *t);
void expand (QTreeWidgetItem *t);
void srch_sub(QTreeWidgetItem *t, const QString &s, QList<QTreeWidgetItem *> &results);
QTreeWidgetItem *srch_cat (QTreeWidgetItem *t, QString s);

void MainWindow::tree_addbranch()
{
QTreeWidgetItem *b;
QString s;

    b = new QTreeWidgetItem (tree);
    tree->addTopLevelItem(b);

    s.clear();
    s.append(tr(""));
    b->setText(0, s);
    s.clear();
    s.append(tr("New Category<p></p>Enter the category description here. The first line becomes the name in the tree on the left."));
    b->setText(1, s);

    cur_branch = b;
    cur_leaf = b;
    catflag = 1;

    tree->setCurrentItem(b);

    show_branch_data ();

    document_modified = true;
    //status text here
    statustext->setText(tr("Added a new category. File modified"));
}

void MainWindow::tree_addsubbranch()
{
QString s;
QTreeWidgetItem *b;

    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("No category present. Add one first."));
        return;
    }
    b = new QTreeWidgetItem ();

    s.clear();
    s.append(tr(""));
    b->setText(0, s);
    s.clear();
    s.append(tr("New Branch<p></p>Enter the branch data here. The first line becomes the name in the tree on the left."));
    b->setText(1, s);

    if (catflag == 0){//subbranch
        cur_leaf->addChild(b);
    }
    else {//category
        cur_branch->addChild(b);
        catflag = 0;
    }

    cur_leaf = b;
    tree->setCurrentItem(b);

    show_branch_data ();
    document_modified = true;
    statustext->setText(tr("Added a new branch. File modified"));
}

void MainWindow::tree_delbranch()
{
int i, j, childcount;
QTreeWidgetItem *b, *deleted;

    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    childcount = (cur_leaf == nullptr) ? 0 : cur_leaf->childCount();
    if (childcount > 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("The branch contains a sub-branch. Delete it first.\n""Click OK to continue"));
        msgBox.exec();
        return;
    }

    if (catflag == 0){//subbranch
        b = cur_leaf->parent();
        if (b == nullptr)
            return;
        deleted = cur_leaf;
        b->removeChild(deleted);
        delete deleted;
        set_branch(b);
        //status text here
        statustext->setText(tr("Deleted sub-branch. File modified"));

        return;
    }
    else {//category
        i = tree->indexOfTopLevelItem(cur_branch);
        if (i < 0)
            return;
        deleted = cur_branch;
        if (i > 0) {
            cur_branch = tree->topLevelItem(i-1);
        }
        else if (tree->topLevelItemCount() > 1) {
            cur_branch = tree->topLevelItem(1);
        }
        else {
            cur_branch = nullptr;
        }

        tree->takeTopLevelItem(i);
        delete deleted;
        statustext->setText(tr("Deleted category. File modified"));
    }
    j = tree->topLevelItemCount();
    if (j > 0) {
        cur_leaf = cur_branch;
        set_branch(cur_branch);
    }
    else {
        cur_branch = nullptr;
        cur_leaf = nullptr;
        statustext->setText(tr("The tree is empty"));
        leafdoc->setHtml("<p></p>");
    }
    catflag = 1;
    document_modified = true;
}

void MainWindow::tree_cutbranch()
{
    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    tree_copybranch();
    tree_delbranch_after_copy();
    document_modified = true;
    //status text here
    statustext->setText(tr("File modified"));
}

void MainWindow::tree_delbranch_after_copy()
{
int i;
QTreeWidgetItem *deleted;

    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    if (catflag == 0){//subbranch
        cur_branch = cur_leaf->parent();
        if (cur_branch == nullptr)
            return;
        deleted = cur_leaf;
        cur_branch->removeChild(deleted);
        delete deleted;
        statustext->setText(tr("Deleted sub-branch. File modified"));
    }
    else {//category
        i = tree->indexOfTopLevelItem(cur_branch);
        if (i < 0)
            return;
        deleted = cur_branch;
        if (i > 0) {
            cur_branch = tree->topLevelItem(i-1);
        }
        else if (tree->topLevelItemCount() > 1) {
            cur_branch = tree->topLevelItem(1);
        }
        else {
            cur_branch = nullptr;
        }

        tree->takeTopLevelItem(i);
        delete deleted;
        statustext->setText(tr("Deleted category. File modified"));
    }
    if (tree->topLevelItemCount() > 0) {
        cur_leaf = cur_branch;
        set_branch(cur_branch);
    }
    else {
        cur_branch = nullptr;
        cur_leaf = nullptr;
        statustext->setText(tr("The tree is empty"));
        leafdoc->setHtml("<p></p>");
    }
    document_modified = true;
    catflag = 1;
}

void MainWindow::tree_copybranch()
{
    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    if (catflag == 0){//subbranch
        if (copy_branch != nullptr)
            delete copy_branch;
        copy_branch = cur_leaf->clone();
        statustext->setText(tr("Copied sub-branch."));
    }
    else {//category
        if (copy_branch != nullptr)
            delete copy_branch;
        copy_branch = cur_branch->clone();
        statustext->setText(tr("Copied category."));
    }
}

void MainWindow::tree_pastebranch()
{
    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    if (copy_branch == nullptr) {
        statustext->setText(tr("Nothing to paste. Copy or cut a branch first."));
        return;
    }

    cur_leaf->insertChild(0, copy_branch);
    copy_branch = nullptr;
    statustext->setText(tr("Copied branch. File modified"));
}

void MainWindow::set_branch (QTreeWidgetItem *b)
//item clicked
{
    if (b == nullptr)
        return;

    if (b->parent() != NULL) {//leaf item
        //leaf and branch are different
        cur_branch = b->parent();
        cur_leaf = b;
        catflag = 0;
    }
    else {//category item
        //leaf and branch are the same
        cur_branch = b;
        cur_leaf = b;
        catflag = 1;
    }

    show_branch_data ();
    tree->setCurrentItem(cur_leaf);
    cur_leaf->setSelected(true);
}

void MainWindow::show_branch_data()
{
QString s;
QByteArray d;
QFont font;

    //set the flag first, so all programmatic writes below (modify_name, the
    //contentsChanged write-back from setHtml) are not counted as user changes
    branch_display_in_progress = true;

    s.clear();
    if (catflag == 0){//subbranch selected
        if (cur_leaf == nullptr){
            branch_display_in_progress = false;
            return;
        }
        s.append(cur_leaf->text(1));
        modify_name (cur_leaf);
    }
    else {//category selected
        if (cur_branch == nullptr){
            branch_display_in_progress = false;
            return;
        }
        s.append(cur_branch->text(1));
        modify_name(cur_branch);
    }

    //shrink oversized data-dir images so their width fits the editor panel
    s = fit_images_to_width(s, DataDir, qMax(60, leafview->viewport()->width() - 30));
    d = s.toUtf8();

    //resolve relative image links (bare filenames) against the data directory
    leafdoc->setBaseUrl (QUrl::fromLocalFile(DataDir + "/"));
    font = comboFont->currentFont ();
    leafdoc->setHtml (d);
    leafview->setDocument (leafdoc);
    leafview->setCurrentFont (font);
    checker.setTextEdit (leafview);

    //make the current search's matches visible in the branch just opened;
    //with an empty search box this simply clears any leftover highlights
    highlight_search(srchbox->text(), search.occRank);

    branch_display_in_progress = false;
}

void MainWindow::highlight_search (const QString &needle, int which)
//mark every occurrence of the search text in the editor; the which-th one
//(0-based, per branch) is emphasised so stepping past several hits in the
//same branch stays visible. The emphasis is also scrolled into view, so the
//current hit is never hidden outside the visible part of the editor.
{
QList<QTextEdit::ExtraSelection> sel;
QTextCharFormat fmtAll, fmtCur;
QTextCursor c, curOcc;
int i;

    if (needle.isEmpty()) {
        leafview->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        return;
    }

    //all matches share a soft background, the current one a stronger one
    fmtAll.setBackground(QColor(255, 255, 0, 120));
    fmtCur.setBackground(QColor(255, 165, 0, 180));

    i = 0;
    c = leafdoc->find(needle, QTextCursor(leafdoc));
    while (c.isNull() == false) {
        if (i == which)
            curOcc = c;
        QTextEdit::ExtraSelection e;
        e.cursor = c;
        e.format = (i == which) ? fmtCur : fmtAll;
        sel.append(e);
        i++;
        c = leafdoc->find(needle, c);
    }
    leafview->setExtraSelections(sel);

    //bring the current occurrence into view: park the caret on the start of
    //that match (no text is selected or changed) and scroll it visible
    if (curOcc.isNull() == false) {
        curOcc.setPosition(curOcc.selectionStart());
        leafview->setTextCursor(curOcc);
        leafview->ensureCursorVisible();
    }
}

void MainWindow::modify_name (QTreeWidgetItem *b)
{
QString s1, s;
QTextDocument doc;
int i;

    s1 = b->text(1);
    doc.setHtml(s1);
    s = doc.toPlainText();

    for (i = 0; i < s.length(); i++){
        if (s[i] == QChar ('\n'))
            s.chop (s.length() - i);
    }

    b->setText(0, s);
}

void MainWindow::get_data_from_leaf()
{
QString s;


    if (file_read_in_progress == true)
        return;
    if (branch_display_in_progress == true)
        return;
    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        //leafdoc->setHtml("<p></p>");
        return;
    }
    s.clear();

    s = leafdoc->toHtml();
    //keep stored branches portable: display sizes are transient, so drop them
    s = strip_image_sizes(s);

    if (catflag == 1) {
        if (cur_branch == nullptr)
            return;
        cur_branch->setText(1, s);
        modify_name(cur_branch);
    }
    else {
        if (cur_leaf == nullptr)
            return;
        cur_leaf->setText(1, s);
        modify_name(cur_leaf);
    }
}

void MainWindow::expand_tree()
{
int i, tlc;
QTreeWidgetItem *t;

    tlc = tree->topLevelItemCount();
    if (tlc == 0) {
        statustext->setText(tr("Nothing to expand"));
        return;
    }

    for (i = 0; i < tlc; i++) {
        t = tree->topLevelItem(i);
        expand (t);
    }
}

void expand (QTreeWidgetItem *t)
{
int i, childcount;

    t->setExpanded (true);
    childcount = t->childCount();
    if (childcount > 0) {
        for (i = 0; i < childcount; i++) {
            expand (t->child(i));
        }
    }
}

void MainWindow::collapse_tree()
{
int i, tlc;
QTreeWidgetItem *t;

    tlc = tree->topLevelItemCount();
    if (tlc == 0) {
        statustext->setText(tr("Nothing to collapse"));
        return;
    }

    for (i = 0; i < tlc; i++) {
        t = tree->topLevelItem(i);
        collapse(t);
    }
}

void collapse (QTreeWidgetItem *t)
{
int i, childcount;

    t->setExpanded (false);
    childcount = t->childCount();
    if (childcount > 0) {
        for (i = 0; i < childcount; i++) {
            collapse (t->child(i));
        }
    }
}

void MainWindow::sort_asc_tree()
{
int tlc;

    tlc = tree->topLevelItemCount();
    if (tlc == 0) {
        statustext->setText(tr("Nothing to sort"));
        return;
    }

    tree->sortItems(0, Qt::AscendingOrder);
    document_modified = true;//the saved order of the tree has changed
}

void MainWindow::sort_desc_tree()
{
int tlc;

    tlc = tree->topLevelItemCount();
    if (tlc == 0) {
        statustext->setText(tr("Nothing to sort"));
        return;
    }

    tree->sortItems(0, Qt::DescendingOrder);
    document_modified = true;//the saved order of the tree has changed
}

void MainWindow::tree_srch_nxt()
{
QTreeWidgetItem *cat;
QString s, s1;
int i, sz;

    catcount = tree->topLevelItemCount();
    if (catcount == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    s = srchbox->text();
    if (s.isEmpty() == true) {
        statustext->setText(tr("Please enter text in search box"));
        return;
    }
    if (s != search.lastText) {
        search.index = 0;
        search.lastText = s;
    }

    for (QTreeWidgetItem *occ : std::as_const(search.results)) {
        occ->setSelected(false);
    }
    search.results.clear();

    //loop over categories
    for (i = 0; i < catcount; i++){
        //next top level category
        cat = tree->topLevelItem(i);
        srch_sub (cat, s, search.results);
    }

    sz = search.results.size();
    if (sz > 0) {
        if (search.index >= sz)
            search.index = 0;
        //which occurrence *within* the branch this one is, so it can be
        //emphasised among the other matches shown in the editor
        search.occRank = 0;
        for (i = 0; i < search.index; i++)
            if (search.results[i] == search.results[search.index])
                search.occRank++;
        set_branch(search.results[search.index]);
        s1 = QString(tr("Displaying %1 of %2 occurences")).arg(search.index+1).arg(sz);
        statustext->setText(s1);
        search.index++;
        if (search.index >= sz)
            search.index = 0;
    }
    else
        statustext->setText("Text not found");
}

void MainWindow::tree_srch_pre()
{
QTreeWidgetItem *cat;
QString s, s1;
int i, sz;

    catcount = tree->topLevelItemCount();
    if (catcount == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    s = srchbox->text();
    if (s.isEmpty() == true) {
        statustext->setText(tr("Please enter text in search box"));
        return;
    }
    if (s != search.lastText) {
        search.index = 0;
        search.lastText = s;
    }

    for (QTreeWidgetItem *occ : std::as_const(search.results)) {
        occ->setSelected(false);
    }
    search.results.clear();

    //loop over categories
    for (i = 0; i < catcount; i++){
        //next top level category
        cat = tree->topLevelItem(i);
        srch_sub (cat, s, search.results);
    }

    sz = search.results.size();
    if (sz > 0) {
        search.index--;
        if (search.index < 0)
            search.index = sz - 1;
        //which occurrence *within* the branch this one is (for the highlight)
        search.occRank = 0;
        for (i = 0; i < search.index; i++)
            if (search.results[i] == search.results[search.index])
                search.occRank++;
        set_branch(search.results[search.index]);
        s1 = QString(tr("Displaying %1 of %2 occurences")).arg(search.index+1).arg(sz);
        statustext->setText(s1);
    }
    else
        statustext->setText(tr("Text not found"));
}

void srch_sub (QTreeWidgetItem *t, const QString &s, QList<QTreeWidgetItem *> &results)
{
QString s1;
QTextDocument doc;
qsizetype occ, i, childcount;

    t->setSelected(false);
    //match against the *visible* text of the branch: image file names, tag
    //names and attributes inside the stored html are not searchable content
    //(the same plain-text conversion the editor shows, so the occurrence count
    //agrees with what is highlighted there)
    doc.setHtml(t->text(1));
    s1 = doc.toPlainText();
    //every occurrence of the string in the branch is a separate search result,
    //so the reported count and the up/down stepping work per occurrence
    occ = s1.count(s, Qt::CaseInsensitive);
    for (i = 0; i < occ; i++)
        results.append(t);
    //go down the sub-branches
    childcount = t->childCount();
    if (childcount > 0) {
        for (i = 0; i < childcount; i++) {
            srch_sub (t->child(i), s, results);
        }
    }
}
