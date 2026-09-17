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

//Last modified Sept 16, 2026

#include "treecle.h"

#include <QRegularExpression>
#include <QSaveFile>
#include <QFileInfo>
#include <QUrl>
#include <functional>

//sanity limits for .trc parsing (survive corrupt or hostile files)
static const qint64 MAX_BRANCH_LENGTH = 50 * 1024 * 1024;
static const int MAX_CATEGORIES = 100000;
static const int MAX_CHILDREN = 100000;
static const int MAX_FORMAT_VERSION = 1;//highest version this build can read
static const char *const TRC_MAGIC = "Treecle .trc file";

void MainWindow::new_file()
{
QString s;

    //common tail: a new file is always the unsaved placeholder name holding
    //one fresh branch, with the editor focused and the modified state clean
    auto startNew = [&]() {
        Currentfile.clear();
        Currentfile.append("Noname.trc");
        release_file_lock();
        tree_addbranch();//the new tree starts with one branch
        document_modified = false;//a fresh document is not dirty
        updateWindowTitle();
        set_editor_focus();
        statustext->setText(tr("New file"));
    };

    if (document_modified == true) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Treecle"), tr("Do you wish to save or discard the current tree?\n"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Cancel) {//user cancels new file operation
            statustext->setText(tr("Tree has not been saved "));
            return;
        }
        if (ret == QMessageBox::Save) {
            if (save_file() == true) {//file saved by the user
                s.append (tr("Saved "));
                s.append (Currentfile);
                delete_tree();
                startNew();
                statustext->setText(s);//keep the "Saved ..." confirmation
            }
            else {//user cancelled the save operation using the cancel button in the file dialog
                statustext->setText(tr("Tree has not been saved"));
            }
            return;
        }
        if (ret == QMessageBox::Discard) {
            delete_tree();
            startNew();
            return;
        }
    }
    else {
        delete_tree();
        startNew();
    }
}

void MainWindow::open_file()
{
QTreeWidgetItem *cat;
QList<QTreeWidgetItem *> tops;
QString fn, s, legacyDir;
bool ok;

    //1. choose the file first: cancelling here leaves the current tree untouched
    fn = QFileDialog::getOpenFileName(this, tr("Open File..."), QString(Openpath), tr("Treecle files (*.trc);;All files (*)"));
    if (fn.isEmpty())
        return;

    //2. if the current tree has unsaved changes, save or discard it
    if (document_modified == true) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Treecle"),
                                   tr("The current tree has not been saved.\nSave it before opening a file?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            if (save_file() == false) {
                statustext->setText(tr("Tree has not been saved"));
                return;//save failed or was cancelled - keep the current tree
            }
        }
        else if (ret == QMessageBox::Cancel)
            return;
        //Discard: drop the current tree only after the new file has loaded
    }

    //3. load the whole file into unattached items first (all-or-nothing)
    QFile file (fn);
    ok = file.open(QFile::ReadOnly);
    if (ok == false) {
        statustext->setText(tr("Could not open the file for reading: ") + fn);
        return;
    }
    QTextStream in(&file);
    in.setEncoding (QStringConverter::Utf8);

    QFileInfo fi(fn);
    file_read_in_progress = true;

    TrcHeaderStatus hs = read_tree_header(&in, catcount);
    if (hs != TrcHeaderStatus::Ok) {
        file.close();
        file_read_in_progress = false;
        if (hs == TrcHeaderStatus::NewerVersion)
            statustext->setText(tr("This file was written by a newer version of Treecle"));
        else
            statustext->setText(tr("The file is not a valid Treecle file or is corrupt"));
        return;
    }

    if (read_tree(&in, catcount, tops) == false) {
        qDeleteAll(tops);//discard everything loaded so far
        file.close();
        file_read_in_progress = false;
        statustext->setText(tr("The file is corrupt - the tree was not loaded"));
        return;
    }
    file.close();

    //normalize image links: absolute paths pointing at the legacy or current
    //data dir become bare filenames, so the file stays portable; the images
    //are resolved against the data dir again when the branch is displayed
    legacyDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    legacyDir.append("/.treecle");
    std::function<void(QTreeWidgetItem*)> fixImgs = [&](QTreeWidgetItem *it) {
        it->setText(1, normalize_img_links(it->text(1), legacyDir, DataDir));
        for (int c = 0; c < it->childCount(); c++)
            fixImgs(it->child(c));
    };
    for (QTreeWidgetItem *t : tops)
        fixImgs(t);

    //4. take the per-file lock: refuse to open the same file in two live
    //instances (Cancel-only; stale locks are removed automatically). If the
    //file differs from the one currently open, release its lock first so the
    //acquired lock always matches Currentfile
    if (QFileInfo(fn).canonicalFilePath() != QFileInfo(Currentfile).canonicalFilePath()) {
        release_file_lock();
        if (acquire_file_lock(fn) == false) {
            file_read_in_progress = false;
            statustext->setText(tr("The file is already open in another instance"));
            if (Currentfile != "Noname.trc")//re-lock the file still in use
                acquire_file_lock(Currentfile);
            return;//the current tree is left untouched
        }
    }

    //5. the file loaded completely: only now replace the current tree
    delete_tree();
    for (QTreeWidgetItem *t : tops)
        tree->addTopLevelItem(t);

    file_read_in_progress = false;
    Currentfile = fn;
    document_modified = false;
    updateWindowTitle();
    Openpath = fi.path();//remember this folder for the next dialog
    //restore the images this document has made copies of, so a removal in a
    //later session can still be cleaned up
    loadOwnedImages(fn);

    cat = tree->topLevelItem(0);
    if (cat == nullptr)
        statustext->setText(tr("The file contains no data"));
    else {
        set_branch(cat);
        s = tr("Read ") + fn;
        statustext->setText(s);
    }
    tree->setFocus();
}

//read exactly `units` UTF-16 code units into *out. QTextStream::read() counts
//Unicode characters, but branch lengths are written with QString::length()
//(UTF-16 units); without this, astral-plane characters (surrogate pairs, e.g.
//emoji) would under-consume and corrupt the stream. Reading in a loop and
//redeeming the actually-consumed units keeps both sides consistent. A short
//file (fewer characters than promised) is rejected.
static bool read_n_units(QTextStream *in, qint64 units, QString *out)
{
    out->clear();
    while (units > 0) {
        QString chunk = in->read(units);
        if (in->status() != QTextStream::Ok || chunk.isEmpty())
            return false;
        out->append(chunk);
        units -= (qint64)chunk.length();
    }
    return true;
}

bool MainWindow::read_this_branch (QTreeWidgetItem *cat, QTextStream *in)
{
QTreeWidgetItem *leaf;
QString s;
qint64 i;
int j = 0, childnum = 0;

    //length of branch name
    *in >> j;
    if (in->status() != QTextStream::Ok || j < 0 || j > MAX_BRANCH_LENGTH)
        return false;
    //go to next line (otherwise the CR becomes part of the string s below)
    s = in->readLine();
    i = (qint64)j;

    //read the name of this branch
    if (read_n_units(in, i, &s) == false)
        return false;
    cat->setText(0, s);

    //length of branch data
    *in >> j;
    if (in->status() != QTextStream::Ok || j < 0 || j > MAX_BRANCH_LENGTH)
        return false;
    //go to next line (otherwise the CR becomes part of the string s below)
    s = in->readLine();
    i = (qint64)j;

    //read the data in this branch
    if (read_n_units(in, i, &s) == false)
        return false;
    cat->setText(1, s);

    //read the no. of children in this branch
    *in >> childnum;
    if (in->status() != QTextStream::Ok || childnum < 0 || childnum > MAX_CHILDREN)
        return false;

    //loop over children, if any
    if (childnum > 0) {
        for (i = 0; i < childnum; i++){
            leaf = new QTreeWidgetItem (cat);
            cat->addChild(leaf);
            if (read_this_branch (leaf, in) == false)
                return false;
        }
    }
    return (in->status() == QTextStream::Ok);
}

bool MainWindow::write_tree (QTreeWidget *tree, QTextStream *out)
{
QTreeWidgetItem *cat;
int i, catcount;

    catcount = tree->topLevelItemCount();
    *out << TRC_MAGIC << "\n";
    *out << MAX_FORMAT_VERSION << "\n";
    *out << catcount << "\n";

    //loop over categories
    for (i = 0; i < catcount; i++){
        //next top level category
        cat = tree->topLevelItem(i);
        save_this_branch (cat, out);
    }

    out->flush();
    return (out->status() == QTextStream::Ok);
}

TrcHeaderStatus MainWindow::read_tree_header (QTextStream *in, int &catcount)
{
QString line1, vline;
bool hdr_ok = false, vok = false;
int v;

    //parse the optional magic line + format version; legacy .trc files start
    //directly with the category count and remain readable
    line1 = in->readLine();
    if (line1 == QLatin1String(TRC_MAGIC)) {
        vline = in->readLine();
        v = vline.trimmed().toInt(&vok);
        if (in->status() != QTextStream::Ok || vok == false || v < 1 || v > MAX_FORMAT_VERSION) {
            if (vok && v > MAX_FORMAT_VERSION)
                return TrcHeaderStatus::NewerVersion;
            return TrcHeaderStatus::Corrupt;
        }
        *in >> catcount;
        hdr_ok = true;
    }
    else
        catcount = line1.trimmed().toInt(&hdr_ok);
    if (in->status() != QTextStream::Ok || hdr_ok == false ||
        catcount < 0 || catcount > MAX_CATEGORIES)
        return TrcHeaderStatus::Corrupt;
    return TrcHeaderStatus::Ok;
}

bool MainWindow::read_tree (QTextStream *in, int catcount, QList<QTreeWidgetItem *> &tops)
{
QTreeWidgetItem *top;
int i;

    for (i = 0; i < catcount && in->status() == QTextStream::Ok; i++){
        top = new QTreeWidgetItem();
        tops.append(top);
        if (read_this_branch (top, in) == false)
            return false;
    }
    return (in->status() == QTextStream::Ok);
}

bool MainWindow::save_file()
{
QString s;
bool ok;

    if (tree->topLevelItemCount() == 0 || document_modified == false) {
        statustext->setText(tr("Nothing to save"));
        return false;
    }

    if (Currentfile == "Noname.trc") {
        return save_file_as();
    }

    //preserve the version about to be replaced: the .bak always holds the
    //immediately-previous save, so a later bad save is never fatal
    make_backup_copy(Currentfile);

    QSaveFile file (Currentfile);

    ok = file.open(QFile::WriteOnly);
    if (ok == false) {
        statustext->setText(tr("Could not open the file for writing: ") + Currentfile);
        return false;
    }

    QTextStream out(&file);
    //out.setCodec("UTF-8");
    out.setEncoding (QStringConverter::Utf8);

    if (write_tree(tree, &out) == false || file.error() != QFileDevice::NoError) {
        file.cancelWriting();
        statustext->setText(tr("Failed to write the file: ") + Currentfile);
        return false;
    }
    if (file.commit() == false) {//atomic replace; the original is preserved on failure
        statustext->setText(tr("Could not save the file: ") + Currentfile);
        return false;
    }

    s.append (tr("Saved "));
    s.append (Currentfile);
    statustext->setText(s);
    document_modified = false;
    updateWindowTitle();
    gcOrphanedImages();
    return true;
}

bool MainWindow::save_file_as()
{
QString s;
QString prevFile;
bool ok;

    //remember the file the ownership sidecar currently belongs to, so a Save
    //As to a different path can drop the now-stale sidecar (its key changes)
    prevFile = Currentfile;

    QString fn = QFileDialog::getSaveFileName(this, tr("Save File..."), QString(Openpath), tr("Treecle files (*.trc);;All files (*)"));
    if (fn.isEmpty())
        return false;
    QFileInfo fi(fn);
    if (fi.suffix().isEmpty())
        fn.append(".trc");
    fi = QFileInfo(fn);

    //if the target file differs from the one currently open, switch the
    //per-file lock to it BEFORE writing (refusing to overwrite a file that
    //another live instance holds is the point of the lock); a failed write
    //restores the lock of the file still open
    if (QFileInfo(fn).canonicalFilePath() != QFileInfo(Currentfile).canonicalFilePath()) {
        release_file_lock();
        if (acquire_file_lock(fn) == false) {
            statustext->setText(tr("That file is already open in another instance - nothing was saved"));
            if (Currentfile != "Noname.trc")//re-lock the file still in use
                acquire_file_lock(Currentfile);
            return false;
        }
    }

    //when we overwrite an existing file, keep its current version as .bak
    //(a Save As to a brand-new path simply has nothing to back up)
    make_backup_copy(fn);

    QSaveFile file (fn);

    ok = file.open(QFile::WriteOnly);
    if (ok == false) {
        statustext->setText(tr("Could not open the file for writing: ") + fn);
        if (QFileInfo(fn).canonicalFilePath() != QFileInfo(Currentfile).canonicalFilePath()) {
            release_file_lock();
            if (Currentfile != "Noname.trc")//re-lock the file still in use
                acquire_file_lock(Currentfile);
        }
        return false;
    }

    QTextStream out(&file);
    //out.setCodec("UTF-8");
    out.setEncoding (QStringConverter::Utf8);

    if (write_tree(tree, &out) == false || file.error() != QFileDevice::NoError) {
        file.cancelWriting();
        statustext->setText(tr("Failed to write the file: ") + fn);
        if (QFileInfo(fn).canonicalFilePath() != QFileInfo(Currentfile).canonicalFilePath()) {
            release_file_lock();
            if (Currentfile != "Noname.trc")//re-lock the file still in use
                acquire_file_lock(Currentfile);
        }
        return false;
    }
    if (file.commit() == false) {
        statustext->setText(tr("Could not save the file: ") + fn);
        if (QFileInfo(fn).canonicalFilePath() != QFileInfo(Currentfile).canonicalFilePath()) {
            release_file_lock();
            if (Currentfile != "Noname.trc")//re-lock the file still in use
                acquire_file_lock(Currentfile);
        }
        return false;
    }

    s.append (tr("Saved "));
    s.append (fn);
    statustext->setText(s);
    Currentfile = fn;
    document_modified = false;
    updateWindowTitle();
    Openpath = fi.path();//remember this folder for the next dialog
    gcOrphanedImages();
    //the sidecar follows the new file key; drop the stale one if the previous
    //document was a real file that now lives under a different canonical path
    if (prevFile != "Noname.trc" &&
        QFileInfo(prevFile).canonicalFilePath() != QFileInfo(fn).canonicalFilePath())
        QFile::remove(image_own_name(DataDir, prevFile));

    return true;
}

QString normalize_img_links (const QString &html, const QString &legacyDir, const QString &dataDir)
{
QRegularExpression re("<img\\s[^>]*?src=\"([^\"]*)\"[^>]*>");
QRegularExpressionMatchIterator it;
QRegularExpressionMatch m;
QStringList dirs;
QString out, path, bare, src;
int pos = 0;

    dirs << legacyDir << dataDir;

    //rebuild html, replacing only the src value of matched tags; indexes stay
    //relative to the original string so the bare-name splice is always correct
    it = re.globalMatch(html);
    while (it.hasNext()) {
        m = it.next();
        src = m.captured(1);
        path = QUrl::fromPercentEncoding(src.toUtf8());//decode %20 for comparison
        bare.clear();
        for (const QString &d : dirs) {
            if (d.isEmpty())
                continue;
            if (path.startsWith(d + "/")) {
                bare = QFileInfo(src).fileName();//keep the encoded basename
                break;
            }
        }
        if (bare.isEmpty() == false) {
            out.append(html.mid(pos, m.capturedStart(1) - pos));
            out.append(bare);
            pos = m.capturedEnd(1);
        }
    }
    out.append(html.mid(pos));
    return out;
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
    clear_search_state();

    cur_branch = nullptr;
    cur_leaf = nullptr;
    catflag = 1;
    leafdoc->setHtml("<p></p>");
    //the old document is gone, so its image copies are no longer owned by the
    //current document and must never be deleted by a later save (the sidecar
    //the document itself got is restored by loadOwnedImages() after any switch)
    ownedImages.clear();
    //status text here
    statustext->setText(tr("File modified"));
}
