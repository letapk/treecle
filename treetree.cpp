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

//Last modified Oct 23, 2026

#include "treecle.h"

#include <QHash>
#include <QTextCursor>
#include <QTextCharFormat>

#include <utility>

void collapse (QTreeWidgetItem *t);
void expand (QTreeWidgetItem *t);
void srch_sub(QTreeWidgetItem *t, const QString &s, QList<QTreeWidgetItem *> &results);
QTreeWidgetItem *srch_cat (QTreeWidgetItem *t, QString s);

//nesting level of an item (top level = 1), walked iteratively so measuring
//never recurses even on a tree at the depth limit
static int item_depth (QTreeWidgetItem *item)
{
int d = 0;

    for (QTreeWidgetItem *i = item; i != nullptr; i = i->parent())
        d++;
    return d;
}

//height of a subtree (the root itself counts as 1), also iterative
static int subtree_height (QTreeWidgetItem *root)
{
struct Frame { QTreeWidgetItem *item; int level; };
QList<Frame> stack;
int h = 0;

    if (root == nullptr)
        return 0;
    stack.append({root, 1});
    while (stack.isEmpty() == false) {
        Frame cur = stack.takeLast();
        if (cur.level > h)
            h = cur.level;
        for (int c = 0; c < cur.item->childCount(); c++)
            stack.append({cur.item->child(c), cur.level + 1});
    }
    return h;
}

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

    set_document_modified();
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
    QTreeWidgetItem *parentItem = (catflag == 0) ? cur_leaf : cur_branch;
    if (parentItem == nullptr) {
        statustext->setText(tr("No category present. Add one first."));
        return;
    }
    //refuse to grow past the format's nesting cap: the .trc reader rejects
    //deeper trees, so an unbounded add would produce an unloadable file
    if (item_depth(parentItem) >= MAX_TREE_DEPTH) {
        statustext->setText(tr("Maximum branch nesting depth reached. Nothing was added."));
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
    set_document_modified();
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
        //defensive: the invariant is catflag == 0 => cur_leaf != nullptr, but
        //never dereference a stale/null selection if it is ever violated
        if (cur_leaf == nullptr)
            return;
        b = cur_leaf->parent();
        if (b == nullptr)
            return;
        deleted = cur_leaf;
        b->removeChild(deleted);
        delete deleted;
        clear_search_state();
        set_branch(b);
        set_document_modified();
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
        clear_search_state();
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
    set_document_modified();
}

void MainWindow::tree_cutbranch()
{
    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    tree_copybranch();
    tree_delbranch_after_copy();
    set_document_modified();
    //status text here
    statustext->setText(tr("File modified"));
}

void MainWindow::tree_delbranch_after_copy()
{
int i;
bool blocked;
QTreeWidgetItem *deleted, *parent, *next;

    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    if (catflag == 0){//subbranch
        //capture the parent before removing: removing the selected item emits
        //itemSelectionChanged, and get_highlighted_branch() would otherwise
        //rewrite cur_branch/cur_leaf while this function is still using them
        parent = (cur_leaf == nullptr) ? nullptr : cur_leaf->parent();
        if (parent == nullptr)
            return;
        deleted = cur_leaf;
        blocked = tree->blockSignals(true);
        parent->removeChild(deleted);
        delete deleted;
        tree->setCurrentItem(parent);
        tree->blockSignals(blocked);
        clear_search_state();
        set_branch(parent);//catflag follows the actual parent, not always 1
        set_document_modified();
        statustext->setText(tr("Deleted sub-branch. File modified"));
        return;
    }

    //category: pick the neighbour that becomes current *before* removing, so
    //the selection change cannot rewrite it midway through the operation
    i = tree->indexOfTopLevelItem(cur_branch);
    if (i < 0)
        return;
    deleted = cur_branch;
    if (i > 0)
        next = tree->topLevelItem(i-1);
    else if (tree->topLevelItemCount() > 1)
        next = tree->topLevelItem(1);
    else
        next = nullptr;

    blocked = tree->blockSignals(true);
    tree->takeTopLevelItem(i);
    delete deleted;
    if (next != nullptr)
        tree->setCurrentItem(next);
    tree->blockSignals(blocked);
    clear_search_state();
    if (next != nullptr) {
        set_branch(next);
        statustext->setText(tr("Deleted category. File modified"));
    }
    else {
        cur_branch = nullptr;
        cur_leaf = nullptr;
        catflag = 1;
        statustext->setText(tr("The tree is empty"));
        leafdoc->setHtml("<p></p>");
    }
    set_document_modified();
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
    if (copy_branch == nullptr) {
        if (tree->topLevelItemCount() == 0)
            statustext->setText(tr("The tree is empty"));
        else
            statustext->setText(tr("Nothing to paste. Copy or cut a branch first."));
        return;
    }

    //cutting the last branch leaves its clone in the clipboard even though the
    //tree is empty: it can come back as a new top-level category
    if (tree->topLevelItemCount() == 0) {
        QTreeWidgetItem *pasted;
        if (subtree_height(copy_branch) > MAX_TREE_DEPTH) {
            statustext->setText(tr("Pasting would exceed the maximum nesting depth. Nothing was pasted."));
            return;
        }
        pasted = copy_branch;
        copy_branch = nullptr;
        tree->addTopLevelItem(pasted);
        set_branch(pasted);
        set_document_modified();
        statustext->setText(tr("Copied branch. File modified"));
        return;
    }

    //refuse a paste that would push any part of the pasted subtree past the
    //format's nesting cap (the reader would reject the saved file later); the
    //copy is kept, so pasting it somewhere shallower still works
    if (item_depth(cur_leaf) + subtree_height(copy_branch) > MAX_TREE_DEPTH) {
        statustext->setText(tr("Pasting here would exceed the maximum nesting depth. Nothing was pasted."));
        return;
    }

    cur_leaf->insertChild(0, copy_branch);
    copy_branch = nullptr;
    set_document_modified();
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
    set_document_modified();//the saved order of the tree has changed
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
    set_document_modified();//the saved order of the tree has changed
}

void MainWindow::clear_search_state()
//drop raw pointers from the results list before any structural change (delete,
//open, new, cut) that destroys tree items; callers are free to also clear
//lastText when the search query should be forgotten entirely
{
    search.results.clear();
    search.shown = -1;
    search.lastText.clear();
    search.occRank = 0;
}

void MainWindow::cancel_search()
//stop a running search midway: forget the results and the query text, clear
//the search box and drop the match highlights in the editor. Triggered by ESC
//from anywhere, or by any key/mouse interaction outside the search controls
//(see eventFilter); a structural change (clear_search_state) already ends a
//search's stepping, and this ends the query text and highlights too
{
    //no search is running; the early return also blocks a re-entrant call,
    //since clear_search_state() empties lastText
    if (search.lastText.isEmpty())
        return;

    clear_search_state();
    srchbox->clear();
    //an empty needle clears the leftover highlight marks (the branch stays
    //displayed as it was, only the search marks go)
    highlight_search(QString(), 0);
    statustext->setText(tr("Search stopped"));
}

static void collect_items_in_order(QTreeWidgetItem *t, QList<QTreeWidgetItem *> &out)
//t itself followed by its descendants, parents before children - the same walk
//srch_sub() uses to fill the results, so the two orders always agree
{
    out.append(t);
    int childcount = t->childCount();
    for (int i = 0; i < childcount; i++)
        collect_items_in_order(t->child(i), out);
}

static int search_anchor_index(QTreeWidget *tree, const QList<QTreeWidgetItem *> &results,
                               QTreeWidgetItem *current, bool forward)
//index of the occurrence a brand-new search should show first: the one inside
//`current` itself, or the next (forward) / previous (!forward) one after or
//before it in document order; -1 when there is no current item to anchor on or
//nothing exists on that side (the caller then wraps, or falls back)
{
    QList<QTreeWidgetItem *> order;
    QHash<QTreeWidgetItem *, int> rank;
    qsizetype i, sz, cur;

    if (current == nullptr)
        return -1;

    for (i = 0; i < tree->topLevelItemCount(); i++)
        collect_items_in_order(tree->topLevelItem(i), order);
    for (i = 0; i < order.size(); i++)
        rank.insert(order[i], int(i));

    if (rank.contains(current) == false)
        return -1;
    cur = rank.value(current);

    sz = results.size();
    if (forward) {
        //first occurrence in the current branch or a later one
        for (i = 0; i < sz; i++)
            if (rank.value(results[i], -1) >= cur)
                return int(i);
    }
    else {
        //last occurrence in the current branch or an earlier one
        for (i = sz - 1; i >= 0; i--)
            if (rank.value(results[i], -1) <= cur)
                return int(i);
    }
    return -1;
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
        search.shown = -1;
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
        if (search.shown < 0) {
            //a new search starts from the branch the user is looking at: its
            //first occurrence, or the next one after it (wrapping back to the
            //very first when nothing is shown after the current branch)
            search.shown = search_anchor_index(tree, search.results, tree->currentItem(), true);
            if (search.shown < 0)
                search.shown = 0;
        }
        else {
            //step forward from the currently shown occurrence (wrapping)
            search.shown = (search.shown + 1) % sz;
        }
        //which occurrence *within* the branch this one is, so it can be
        //emphasised among the other matches shown in the editor
        search.occRank = 0;
        for (i = 0; i < search.shown; i++)
            if (search.results[i] == search.results[search.shown])
                search.occRank++;
        set_branch(search.results[search.shown]);
        s1 = QString(tr("Displaying %1 of %2 occurrences. Press ESC to stop")).arg(search.shown+1).arg(sz);
        statustext->setText(s1);
    }
    else
        statustext->setText(tr("Text not found"));
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
        search.shown = -1;
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
        //editing a branch can shrink the result set while the old index is
        //still stored; clamp it before stepping back or indexing the list
        if (search.shown >= sz)
            search.shown = sz - 1;
        if (search.shown < 0) {
            //same idea backwards: begin at the last occurrence in the branch
            //being looked at, or the one before it (wrapping to the very last
            //when nothing is shown before the current branch)
            search.shown = search_anchor_index(tree, search.results, tree->currentItem(), false);
            if (search.shown < 0)
                search.shown = sz - 1;
        }
        else {
            //step back from the currently shown occurrence (wrapping); from
            //the first one this lands on the last
            search.shown = (search.shown <= 0) ? sz - 1 : search.shown - 1;
        }
        //which occurrence *within* the branch this one is (for the highlight)
        search.occRank = 0;
        for (i = 0; i < search.shown; i++)
            if (search.results[i] == search.results[search.shown])
                search.occRank++;
        set_branch(search.results[search.shown]);
        s1 = QString(tr("Displaying %1 of %2 occurrences. Press ESC to stop")).arg(search.shown+1).arg(sz);
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
