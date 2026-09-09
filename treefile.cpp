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

//sanity limits for .trc parsing (survive corrupt or hostile files)
static const qint64 MAX_BRANCH_LENGTH = 50 * 1024 * 1024;
static const int MAX_CATEGORIES = 100000;
static const int MAX_CHILDREN = 100000;

int MainWindow::new_file()
{
int i;
QString s;

    if (file_modified == true) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Treecle"), tr("Do you wish to save or discard the current tree?\n"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Cancel) {//user cancels new file operation
            statustext->setText(tr("Tree has not been saved "));
            return 1;
        }
        if (ret == QMessageBox::Save) {
            i = save_file();
            if (i == 1) {//file saved by the user
                s.append (tr("Saved "));
                s.append (Currentfile);
                statustext->setText(s);
                delete_tree();
                tree->setHeaderLabel("Filename");
                Currentfile.clear();
                Currentfile.append("Noname.trc");
                file_modified = false;
            }
            else {//user cancelled the save operation using the cancel button in the file dialog
                statustext->setText(tr("Tree has not been saved"));
            }
            return 0;
        }
        if (ret == QMessageBox::Discard) {
            delete_tree();
            tree->setHeaderLabel("Filename");
            statustext->setText(tr("Tree discarded"));
            file_modified = false;
            return 0;
        }
    }
    else {
        delete_tree();
        tree->setHeaderLabel("Filename");
        statustext->setText(tr("New file"));
        Currentfile.clear();
        Currentfile.append("Noname.trc");
        file_modified = false;
    }

    return 0;
}

void MainWindow::open_file()
{
QTreeWidgetItem *cat;
QString s;
int i;
bool ok;

    i = new_file ();//to save current tree if any
    if (i == 1)//operation cancelled
        return;

    QString fn = QFileDialog::getOpenFileName(this, tr("Open File..."), QString(Homepath), tr("Treecle files (*.trc);;All files (*)"));
    QFile file (fn);

    ok = file.open(QFile::ReadOnly);
    if (ok == false)
        return;
    QTextStream in(&file);
    //in.setCodec("UTF-8");
    in.setEncoding (QStringConverter::Utf8);

    QFileInfo fi(fn);
    file_read_in_progress = true;
    in >> catcount;
    if (in.status() != QTextStream::Ok || catcount < 0 || catcount > MAX_CATEGORIES) {
        file.close();
        delete_tree();
        file_read_in_progress = false;
        file_modified = false;
        statustext->setText(tr("The file is not a valid Treecle file or is corrupt"));
        return;
    }
    //loop over categories
    for (i = 0; i < catcount; i++){
        //create new top level category
        cat = new QTreeWidgetItem (tree);
        tree->addTopLevelItem(cat);
        read_this_branch (cat, &in);
    }

    file.close();

    cat = tree->topLevelItem(0);
    if (cat == nullptr) {
        statustext->setText(tr("The file contains no data"));
        Currentfile = fn;
        file_read_in_progress = false;
        file_modified = false;
        return;
    }
    set_branch(cat);
    tree->setHeaderLabel(fi.fileName());

    s.append (tr("Read "));

    s.append (fn);
    statustext->setText(s);
    Currentfile = fn;
    file_read_in_progress = false;
    file_modified = false;
    tree->setFocus ();

}

void MainWindow::read_this_branch (QTreeWidgetItem *cat, QTextStream *in)
{
QTreeWidgetItem *leaf;
QString s;
qint64 i;
int j = 0, childnum = 0;

    //length of branch name
    *in >> j;
    if (in->status() != QTextStream::Ok || j < 0 || j > MAX_BRANCH_LENGTH)
        return;
    //go to next line (otherwise the CR becomes part of the string s below)
    s = in->readLine();
    i = (qint64)j;

    //read the name of this branch
    s.clear();
    s = in->read(i);
    cat->setText(0, s);

    //length of branch data
    *in >> j;
    if (in->status() != QTextStream::Ok || j < 0 || j > MAX_BRANCH_LENGTH)
        return;
    //go to next line (otherwise the CR becomes part of the string s below)
    s = in->readLine();
    i = (qint64)j;

    //read the data in this branch
    s.clear();
    s = in->read(i);
    cat->setText(1, s);

    //read the no. of children in this branch
    *in >> childnum;
    if (in->status() != QTextStream::Ok || childnum < 0 || childnum > MAX_CHILDREN)
        return;

    //loop over children, if any
    if (childnum > 0) {
        for (i = 0; i < childnum; i++){
            leaf = new QTreeWidgetItem (cat);
            cat->addChild(leaf);
            read_this_branch (leaf, in);
        }
    }
}

int MainWindow::save_file()
{
QTreeWidgetItem *cat;
QString s;
int i;
bool ok;

    if (tree->topLevelItemCount() == 0 || file_modified == false) {
        statustext->setText(tr("Nothing to save"));
        return 0;
    }

    if (Currentfile == "Noname.trc") {
        return save_file_as();
    }
    /*
    QString fn = QFileDialog::getSaveFileName(this, tr("Save File..."), QString(Homepath), tr("Treecle files (*.trc);;All files (*)"));
    QFileInfo fi(fn);
    if (fn.isEmpty())
        return 0;
    if (!fn.isEmpty()) {
        if (fi.suffix().isEmpty())
            fn.append(".trc");
    }
    */
    QFile file (Currentfile);
    QFileInfo fi(Currentfile);

    ok = file.open(QFile::WriteOnly);
    if (ok == false)
        return 0;

    QTextStream out(&file);
    //out.setCodec("UTF-8");
    out.setEncoding (QStringConverter::Utf8);

    catcount = tree->topLevelItemCount();
    out << catcount << "\n";

    //loop over categories
    for (i = 0; i < catcount; i++){
        //next top level category
        cat = tree->topLevelItem(i);
        save_this_branch (cat, &out);
    }

    file.close();

    tree->setHeaderLabel(fi.fileName());

    s.append (tr("Saved "));
    s.append (Currentfile);
    statustext->setText(s);
    file_modified = false;
    return 1;
}

int MainWindow::save_file_as()
{
QTreeWidgetItem *cat;
QString s;
int i;
bool ok;

    QString fn = QFileDialog::getSaveFileName(this, tr("Save File..."), QString(Homepath), tr("Treecle files (*.trc);;All files (*)"));
    QFileInfo fi(fn);
    if (fn.isEmpty())
        return 0;
    if (!fn.isEmpty()) {
        if (fi.suffix().isEmpty())
            fn.append(".trc");
    }
    QFile file (fn);

    ok = file.open(QFile::WriteOnly);
    if (ok == false)
        return 0;

    QTextStream out(&file);
    //out.setCodec("UTF-8");
    out.setEncoding (QStringConverter::Utf8);

    catcount = tree->topLevelItemCount();
    out << catcount << "\n";

    //loop over categories
    for (i = 0; i < catcount; i++){
        //next top level category
        cat = tree->topLevelItem(i);
        save_this_branch (cat, &out);
    }

    file.close();

    tree->setHeaderLabel(fi.fileName());

    s.append (tr("Saved "));
    s.append (fn);
    statustext->setText(s);
    Currentfile = fn;
    file_modified = false;

    return 1;
}

void MainWindow::save_this_branch (QTreeWidgetItem *cat, QTextStream *out)
{
QTreeWidgetItem *leaf;
QString s;
int i, childnum;

    //branch name
    s.clear();
    s.append (cat->text(0));
    //length
    *out << s.length() << "\n";
    //name
    *out << s << "\n";

    //branch data
    s.clear();
    s.append (cat->text(1));
    //length
    *out << s.length() << "\n";
    //data
    *out << s << "\n";

    childnum= cat->childCount();
    //save the no. of children in this branch
    *out << childnum << "\n";

    //loop over the children, if any
    if (childnum > 0) {
        for (i = 0; i < childnum; i++){
            leaf = cat->child(i);
            save_this_branch (leaf, out);
        }
    }
}

void MainWindow::delete_tree ()
{
    tree->clear();

    cur_branch = nullptr;
    cur_leaf = nullptr;
    catflag = 1;
    leafdoc->setHtml("<p></p>");
    //status text here
    statustext->setText(tr("File modified"));
}
