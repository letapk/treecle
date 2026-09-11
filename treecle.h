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

//Last modified Sept 11, 2026

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QApplication>
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QMenuBar>
#include <QSplitter>
#include <QFileDialog>
#include <QToolBar>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QTextBrowser>
#include <QLabel>
#include <QCloseEvent>
#include <QFontComboBox>
#include <QColorDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QStatusBar>
#include <QImageReader>
#include <QMouseEvent>
#include <QLineEdit>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>
#include <QDesktopServices>
#include <QUrl>
#include <QActionGroup>
#include <QFontDatabase>
#include <QShortcut>
#include "QtSpell.hpp"

//free helper functions
class QLockFile;
void check_and_make_data_dir (const QString &dataDir);
bool acquire_lock(QLockFile *lock);
//copy the contents of the legacy ~/.treecle data dir into the standard one
//(returns true if anything was actually moved/copied)
bool migrate_old_data_dir (const QString &oldDir, const QString &newDir);
//rewrite absolute <img src> paths that live under a known data dir to bare
//filenames, so saved files stay portable (resolved against DataDir on show)
QString normalize_img_links (const QString &html, const QString &legacyDir, const QString &dataDir);
//shrink oversized data-dir images so their width fits the editor; adds
//width/height attributes to the html (aspect ratio preserved)
QString fit_images_to_width (const QString &html, const QString &dataDir, int maxWidth);
//remove width/height attributes from <img> tags, so saved branch text keeps
//only portable bare-filename links (display sizes are always recomputed)
QString strip_image_sizes (const QString &html);

//result of parsing the tree-file header (magic line + format version)
enum class TrcHeaderStatus { Ok, Corrupt, NewerVersion };

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QMenu *filemenu = nullptr, *helpmenu = nullptr, *treemenu = nullptr;

    //toolbar
    QToolBar *tb1 = nullptr, *tb2 = nullptr;

    //some global actions needed in other methods
    QAction *actionTextBold = nullptr, *actionTextColor = nullptr, *actionTextItalic = nullptr, *actionTextUnderline = nullptr;
    QAction *actionAlignLeft = nullptr, *actionAlignCenter = nullptr, *actionAlignRight = nullptr, *actionAlignJustify = nullptr;
    QAction *actionInsertImage = nullptr;

    QFontComboBox *comboFont = nullptr;
    QComboBox *comboSize = nullptr;
    QLineEdit *srchbox = nullptr;

    QSplitter *splitter = nullptr;
    QTreeWidget *tree = nullptr;
    QTreeWidgetItem *cur_branch = nullptr, *cur_leaf = nullptr;
    QTreeWidgetItem *copy_branch = nullptr;
    QList<QTreeWidgetItem *> srchlst;
    int srch_idx = 0;
    QString last_search_text;

    QTextEdit *leafview = nullptr;
    QTextDocument *leafdoc = nullptr;

    //whether current branch is a top level category or a child, no. of categories
    int catflag = 0, catcount = 0;

    QLabel *statustext = nullptr;

    QString Gnugplfilename;
    QString Helpfilename;
    //path to the user data directory (images, help PDF, COPYING)
    QString DataDir;
    //directory the open/save dialogs start in (last used folder; on first run
    //falls back to cwd, Documents, home, then the data dir)
    QString Openpath;
    QString Currentfile;
    QtSpell::TextEditChecker checker;

    QShortcut *panelshortcut = nullptr;

    bool file_read_in_progress = false, file_modified = false, branch_display_in_progress = false;

public:
    MainWindow(QWidget *parent = nullptr, const QString &dataDir = QString());
    ~MainWindow();

public slots:
    void setup_menu_and_toolbar ();

    //file menu
    void open_file ();
    void new_file();
    bool save_file();
    bool save_file_as ();

    //persistence primitives (pure file format, testable without dialogs)
    bool write_tree(QTreeWidget *tree, QTextStream *out);
    TrcHeaderStatus read_tree_header(QTextStream *in, int &catcount);
    bool read_tree(QTextStream *in, int catcount, QList<QTreeWidgetItem *> &tops);

    //modification flag access (document_modified)
    bool isModified() const { return file_modified; }
    void setModified(bool m) { file_modified = m; }
    //directory the open/save dialogs start in (persistence primitives)
    QString openDir() const { return Openpath; }
    void quit();

    //help menu
    void help();
    void about();

    //tree menu
    void tree_addbranch();
    void tree_addsubbranch();

    void tree_cutbranch();
    void tree_copybranch();
    void tree_pastebranch();
    void tree_delbranch_after_copy();

    void tree_srch_nxt();
    void tree_srch_pre();
    void tree_delbranch();

    //tree related data
    void modify_name (QTreeWidgetItem *b);
    void set_branch (QTreeWidgetItem *b);
    void get_data_from_leaf();
    void save_this_branch (QTreeWidgetItem *cat, QTextStream *out);
    bool read_this_branch (QTreeWidgetItem *cat, QTextStream *in);

    void delete_tree();

    void expand_tree();
    void collapse_tree();

    void sort_asc_tree();
    void sort_desc_tree();

    //editor toolbar
    void textBold();
    void textItalic();
    void textUnderline();
    void textAlign(QAction *a);
    void textColor();
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void colorChanged(const QColor &c);

    void fontFamily();
    void textSize(int index);

    void insertImage();

    //virtual slots
    void closeEvent(QCloseEvent *event);
    //re-fit on-screen images when the editor panel is resized
    bool eventFilter(QObject *obj, QEvent *ev);

    //load data in editor
    void show_branch_data ();
    //rescale on-screen images so their width fits the editor (display only;
    //rerun when the editor is resized so images follow the panel width)
    void fit_editor_images (int maxWidth);

    //preferences
    void writeprefs();
    void readprefs();

    void set_panel_focus();
    void set_editor_focus();
    void set_tree_focus();
    QTreeWidgetItem* get_highlighted_branch();
    void set_modified_flag();

};

#endif // MAINWINDOW_H
