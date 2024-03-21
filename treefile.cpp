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

#include "treecle.h"

int MainWindow::new_file()
{
int i;

    fmodified = leafview->isModified();

    if (fmodified == true) {
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
                delete_tree();
                tree->setHeaderLabel("");
                statustext->setText(tr("Tree saved"));
            }
            else {//user cancelled the save operation using the cancel button in the file dialog
                statustext->setText(tr("Tree has not been saved"));
            }
            return 0;
        }
        if (ret == QMessageBox::Discard) {
            delete_tree();
            tree->setHeaderLabel("");
            statustext->setText(tr("Tree discarded"));
            return 0;
        }
    }
    else {
        delete_tree();
        tree->setHeaderLabel("");
        statustext->setText(tr(""));
    }
    fmodified = leafview->isModified();

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
    in.setCodec("UTF-8");

    QFileInfo fi(fn);

    in >> catcount;
    //loop over categories
    for (i = 0; i < catcount; i++){
        //create new top level category
        cat = new QTreeWidgetItem (tree);
        tree->addTopLevelItem(cat);
        read_this_branch (cat, &in);
    }

    file.close();

    cat = tree->topLevelItem(0);
    set_branch(cat);
    show_branch_data ();
    tree->setHeaderLabel(fi.fileName());

    s.append (tr("Read "));
    s.append (fn);
    statustext->setText(s);
    fmodified = false;
}

void MainWindow::read_this_branch (QTreeWidgetItem *cat, QTextStream *in)
{
QTreeWidgetItem *leaf;
QString s;
qint64 i;
int j, childnum;

    //length of branch name
    *in >> j;
    //go to next line (otherwise the CR becomes part of the string s below)
    s = in->readLine();
    i = (qint64)j;

    //read the name of this branch
    s.clear();
    s = in->read(i);
    cat->setText(0, s);

    //length of branch data
    *in >> j;
    //go to next line (otherwise the CR becomes part of the string s below)
    s = in->readLine();
    i = (qint64)j;

    //read the data in this branch
    s.clear();
    s = in->read(i);
    cat->setText(1, s);

    //read the no. of children in this branch
    *in >> childnum;

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

    fmodified = leafview->isModified();

    if (tree->topLevelItemCount() == 0 || fmodified == false) {
        statustext->setText(tr("Nothing to save"));
        return 0;
    }

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
    out.setCodec("UTF-8");


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
    fmodified = false;
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
int i;

    i = tree->topLevelItemCount();
    while (i > 0) {
        tree->takeTopLevelItem(0);
        i = tree->topLevelItemCount();
        leafview->setHtml("<p></p>");
    }
}
