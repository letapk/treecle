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

//Last modified Sept 25, 2024

#include "treecle.h"

void collapse (QTreeWidgetItem *t);
void expand (QTreeWidgetItem *t);
void srch_sub(QTreeWidgetItem *t, const QString &s, QList<QTreeWidgetItem *> &srchlst);
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

    file_modified = true;
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
    file_modified = true;
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
    file_modified = true;
}

void MainWindow::tree_cutbranch()
{
    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    tree_copybranch();
    tree_delbranch_after_copy();
    file_modified = true;
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
    file_modified = true;
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

    d = s.toUtf8();

    font = comboFont->currentFont ();
    leafdoc->setHtml (d);
    leafview->setDocument (leafdoc);
    leafview->setCurrentFont (font);
    checker.setTextEdit (leafview);

    branch_display_in_progress = false;
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
    if (s != last_search_text) {
        srch_idx = 0;
        last_search_text = s;
    }

    foreach (cat, srchlst) {
        cat->setSelected(false);
    }
    srchlst.clear();

    //loop over categories
    for (i = 0; i < catcount; i++){
        //next top level category
        cat = tree->topLevelItem(i);
        srch_sub (cat, s, srchlst);
    }

    sz = srchlst.size();
    if (sz > 0) {
        if (srch_idx >= sz)
            srch_idx = 0;
        set_branch(srchlst[srch_idx]);
        s1 = QString(tr("Displaying %1 of %2 occurences")).arg(srch_idx+1).arg(sz);
        statustext->setText(s1);
    }
    else
        statustext->setText("Text not found");
    srch_idx++;
    if (srch_idx >= sz)
        srch_idx = 0;
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
    if (s != last_search_text) {
        srch_idx = 0;
        last_search_text = s;
    }

    foreach (cat, srchlst) {
        cat->setSelected(false);
    }
    srchlst.clear();

    //loop over categories
    for (i = 0; i < catcount; i++){
        //next top level category
        cat = tree->topLevelItem(i);
        srch_sub (cat, s, srchlst);
    }

    sz = srchlst.size();
    if (sz > 0) {
        set_branch(srchlst[sz - srch_idx - 1]);
        s1 = QString(tr("Displaying %1 of %2 occurences")).arg(sz - srch_idx).arg(sz);
        statustext->setText(s1);
    }
    else
        statustext->setText(tr("Text not found"));
    //printf ("size=%i, idx=%i\n", sz, srch_idx);
    srch_idx++;
    if (srch_idx >= sz)
        srch_idx = 0;
}

void srch_sub (QTreeWidgetItem *t, const QString &s, QList<QTreeWidgetItem *> &srchlst)
{
QString s1;
int i, childcount;

    s1 = t->text(1);
    t->setSelected(false);
    //check current branch
    if (s1.contains(s, Qt::CaseInsensitive) == true) {
        srchlst.append(t);
    }
    //go down the sub-branches
    childcount = t->childCount();
    if (childcount > 0) {
        for (i = 0; i < childcount; i++) {
            srch_sub (t->child(i), s, srchlst);
        }
    }
}
