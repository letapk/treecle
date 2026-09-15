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
#include <QSet>
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
bool check_and_make_data_dir (const QString &dataDir);
QString file_lock_name (const QString &dataDir, const QString &filepath);
QString image_own_name (const QString &dataDir, const QString &filepath);
QSet<QString> load_owned_images (const QString &ownFile);
bool write_owned_images (const QString &ownFile, const QSet<QString> &owned);
bool make_backup_copy (const QString &filepath);
bool is_ignorable_qt_warning (QtMsgType type, const QString &msg);
void install_qt_message_filter ();
bool migrate_old_data_dir (const QString &oldDir, const QString &newDir);
QString normalize_img_links (const QString &html, const QString &legacyDir, const QString &dataDir);
QString fit_images_to_width (const QString &html, const QString &dataDir, int maxWidth);
QString strip_image_sizes (const QString &html);
QString unique_data_dir_name (const QString &dataDir, const QString &fileName);
QSet<QString> collect_image_refs (const QTreeWidget &tree);

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
    struct SearchState {
        QList<QTreeWidgetItem *> results;
        int index = 0;
        QString lastText;
        int occRank = 0;
    } search;

    QTextEdit *leafview = nullptr;
    QTextDocument *leafdoc = nullptr;

    //whether current branch is a top level category or a child, no. of categories
    int catflag = 0, catcount = 0;

    QLabel *statustext = nullptr;

    QString Gnugplfilename;
    QString Helpfilename;
    //path to the user data directory (images, help PDF, COPYING)
    QString DataDir;
    QString Openpath;
    QString Currentfile;
    QLockFile *filelock = nullptr;
    QSet<QString> ownedImages;
    QtSpell::TextEditChecker checker;

    QShortcut *panelshortcut = nullptr;

    bool file_read_in_progress = false, document_modified = false, branch_display_in_progress = false;

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

    //per-file locking
    bool acquire_file_lock (const QString &filepath);
    void release_file_lock();

    void gcOrphanedImages(const QString &filepath = QString());
    void loadOwnedImages(const QString &filepath);
    void recordInsertedImage(const QString &basename);

    bool write_tree(QTreeWidget *tree, QTextStream *out);
    TrcHeaderStatus read_tree_header(QTextStream *in, int &catcount);
    bool read_tree(QTextStream *in, int catcount, QList<QTreeWidgetItem *> &tops);

    bool isModified() const { return document_modified; }
    void setModified(bool m) { document_modified = m; }
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
    void fit_editor_images (int maxWidth);
    void highlight_search (const QString &needle, int which);

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
