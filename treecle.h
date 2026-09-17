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
//on first run in an account the data dir does not exist yet: inform the user
//about its creation and create it (true on success); no-ops once it exists
bool check_and_make_data_dir (const QString &dataDir);
//advisory per-file lock name: SHA-1 of the canonical/absolute file path, so a
//lock for "somefile.trc" lives in the data dir as "<hex>.lck"; unrelated files
//never collide and the name needs no escaping of arbitrary paths
QString file_lock_name (const QString &dataDir, const QString &filepath);
//per-document image-ownership sidecar: same SHA-1 key as the lock file, with a
//".own" suffix. It lists the data-dir image copies this .trc file has made, so
//orphan cleanup can remove exactly those (and nothing another file owns)
QString image_own_name (const QString &dataDir, const QString &filepath);
//load/store the list of image copies owned by a document (an empty owned set
//removes the sidecar again); store is atomic and fails without touching data
QSet<QString> load_owned_images (const QString &ownFile);
bool write_owned_images (const QString &ownFile, const QSet<QString> &owned);
//keep the previous version of a file before it is replaced: copy `filepath` to
//"<filepath>.bak" (the last existing .bak is overwritten, so it always holds
//the immediately-previous version). Returns true when a copy was actually
//made; a missing original is a no-op (the backup is best-effort and never
//blocks the save that follows)
bool make_backup_copy (const QString &filepath);
//true when a runtime Qt message is the benign internal "QTextCursor::setPosition:
//... out of range" warning the external spell-checker can provoke by asking for
//a position just past the very end (typing Return after the last line); the
//app-wide handler installed by install_qt_message_filter() drops exactly those
bool is_ignorable_qt_warning (QtMsgType type, const QString &msg);
//redirect the Qt message handler through a filter that forwards everything
//except is_ignorable_qt_warning() messages to the previous handler (install
//once, very early in main())
void install_qt_message_filter ();
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
//name for a copy of fileName inside the data dir: unique, appending a number
//(_1, _2, ...) when the plain name is already taken, so no insert ever shares
//a physical file with an existing one
QString unique_data_dir_name (const QString &dataDir, const QString &fileName);
//bare data-dir image filenames referenced by <img src> tags in any branch text
//(external URLs and absolute paths are never cleanup candidates)
QSet<QString> collect_image_refs (const QTreeWidget &tree);

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
    //state of the last search kept between Next/Previous presses (results,
    //current position, and what text they apply to). results holds one
    //entry per *occurrence*, so a branch whose text mentions the search
    //string several times appears once per mention
    struct SearchState {
        QList<QTreeWidgetItem *> results;
        int index = 0;
        QString lastText;
        //how many of the branch's matches come before the current one: it is
        //the one emphasised when the branch opens in the editor
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
    //directory the open/save dialogs start in (last used folder; on first run
    //falls back to cwd, Documents, home, then the data dir)
    QString Openpath;
    QString Currentfile;
    //advisory lock for the currently open .trc file (null when Currentfile is
    //the non-file placeholder "Noname.trc"); held for the whole time the file
    //is open, released by release_file_lock() on new/open-other/save-as/close
    QLockFile *filelock = nullptr;
    //bare filenames of the data-dir image copies owned by the current document
    //(loaded from the document's ".own" sidecar when opened, grown by
    //insertImage(), pruned and re-persisted after each successful save).
    //Orphan cleanup only ever touches these, so other documents' images and
    //manually placed files are never deleted
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

    //per-file locking: acquire warns with a Cancel-only dialog if another live
    //instance already holds the lock (stale locks are auto-removed by QLockFile)
    bool acquire_file_lock (const QString &filepath);
    void release_file_lock();

    //orphaned-image cleanup: delete the data-dir image copies owned by the current
    //document (sidecar filepath or Currentfile) that the tree no longer
    //references, then re-persist the owned set; called after a successful save
    //(best-effort: any file/sidecar failure is silently ignored so the save is
    //never blocked; the in-memory owned set is still pruned correctly)
    void gcOrphanedImages(const QString &filepath = QString());
    //restore ownedImages for a file just opened (the open_file() load step)
    void loadOwnedImages(const QString &filepath);
    //record a data-dir image copy as belonging to the current document
    //(insertImage calls this; exposed so the cleanup is testable without dialogs)
    void recordInsertedImage(const QString &basename);

    //persistence primitives (pure file format, testable without dialogs)
    bool write_tree(QTreeWidget *tree, QTextStream *out);
    TrcHeaderStatus read_tree_header(QTextStream *in, int &catcount);
    bool read_tree(QTextStream *in, int catcount, QList<QTreeWidgetItem *> &tops);

    //modification flag access (document_modified)
    bool isModified() const { return document_modified; }
    void setModified(bool m) { document_modified = m; }
    //set document_modified and refresh the window title's "modified" star
    void set_document_modified() { document_modified = true; updateWindowTitle(); }
    //keep the window title in sync: "Treecle - <filename>" plus an asterisk
    //when document_modified is set ("Treecle - Noname.trc *" for a new file)
    void updateWindowTitle();
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
    //drop search results/state so raw pointers are never kept across structural
    //changes (delete, open, new, cut) that destroy tree items
    void clear_search_state();
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
    //highlight every occurrence of the search text in the editor, emphasising
    //the which-th one (0-based per branch); an empty needle clears the marks
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
