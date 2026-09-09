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

//Last modified 9 Sept 2026

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
#include <QTranslator>
#include <QDesktopServices>
#include <QUrl>
#include <QActionGroup>
#include <QFontDatabase>
#include <QShortcut>
#include "QtSpell.hpp"

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
    //stores the path to the data subdirectory
    QString Homepath;
    QString Currentfile;
    QtSpell::TextEditChecker checker;

    QShortcut *panelshortcut = nullptr;

    bool file_read_in_progress = false, file_modified = false, branch_display_in_progress = false;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setup_menu_and_toolbar ();

    //file menu
    void open_file ();
    int new_file();
    int save_file();
    int save_file_as ();
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
    void read_this_branch (QTreeWidgetItem *cat, QTextStream *in);

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

    //load data in editor
    void show_branch_data ();

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
