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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#include <QImageReader>
#include <QMouseEvent>
#include <QLineEdit>
#include <QSettings>
#include <QTranslator>
#include <QtWebKit>
#include <QtWebKitWidgets>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QMenu *filemenu, *helpmenu, *treemenu;

    //toolbar
    QToolBar *tb1, *tb2;

    //some global actions needed in other methods
    QAction *actionTextBold, *actionTextItalic, *actionTextUnderline;
    QAction *actionAlignLeft, *actionAlignCenter, *actionAlignRight, *actionAlignJustify;
    QAction *selectall;

    QFontComboBox *comboFont;
    QComboBox *comboSize;
    QLineEdit *srchbox;

    QSplitter *splitter;
    QTreeWidget *tree;
    QTreeWidgetItem *cur_branch, *cur_leaf;

    //QWebEngineView *leafview;
    QWebView *leafview;

    //whether current branch is a top level category or a child, no. of categories
    int catflag, catcount;
    //whether the data has been altered
    int modified;

    QLabel *statustext;

    QString Gnugplfilename;
    QString Helpfilename;
    //stores the path to the data subdirectory
    QString Homepath;


public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void setup_menu_and_toolbar ();

    //file menu
    void open_file ();
    int new_file();
    int save_file();
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
    void textAlign(QAction*);
    void textColor();

    void fontFamily();
    void textSize();

    void insertImage();

    void adjustActions();

    void select_all();

    //virtual slots
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);

    //load data in editor
    void show_branch_data ();

    //preferences
    void writeprefs();
    void readprefs();
};

#endif // MAINWINDOW_H
