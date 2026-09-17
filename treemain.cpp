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

#if !defined(TREECLE_TEST_BUILD)
int main(int argc, char *argv[])
//start user-interface
{
bool ok = false;
QString dataDir;

    Q_INIT_RESOURCE(treecle);
    //filter the benign "QTextCursor::setPosition ... out of range" warnings the
    //external spell-checker can provoke while typing at the editor's end
    install_qt_message_filter();
    QApplication app(argc, argv);
    app.setApplicationName("treecle");

    QTranslator appTranslator;
    ok = appTranslator.load("treecle_" + QLocale::system().name(), qApp->applicationDirPath());
    if (ok == true)
        app.installTranslator(&appTranslator);

    QTranslator qtTranslator;
    ok = qtTranslator.load("qt_" + QLocale::system().name(), qApp->applicationDirPath());
    if (ok == true)
        app.installTranslator(&qtTranslator);

    //path to the user data directory (Linux: ~/.local/share/treecle)
    dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) {
        QMessageBox::critical (nullptr, "Treecle", "Cannot determine the user data directory.");
        return 1;
    }

    //move any existing files over from the legacy ~/.treecle data directory
    //(images, treeclehelp.pdf, COPYING); the new directory is created here
    QString oldpath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    oldpath.append("/.treecle");
    if (migrate_old_data_dir(oldpath, dataDir)) {
        QMessageBox::information (nullptr, "Treecle",
            "Your data files were moved from\n" + oldpath + "\n\nto\n" + dataDir +
            "\nto comply with the standard data directory layout.");
    }

    //first run in this account: the data dir does not exist yet. Inform the
    //user about its creation (images, help PDF, COPYING and file locks are
    //kept there) and create it. On a legacy account the migration above has
    //already made it, so nothing is shown here again.
    if (check_and_make_data_dir(dataDir) == false) {
        QMessageBox::critical (nullptr, "Treecle",
            QObject::tr("Could not create the data directory: ") + dataDir);
        return 1;
    }

    MainWindow mainwindow(nullptr, dataDir);
    mainwindow.show();
    //item: the program starts with a fresh document (one branch, editor focus)
    mainwindow.new_file();

    return app.exec();
}
#endif//TREECLE_TEST_BUILD excludes main() so the tests can link this file

MainWindow::MainWindow(QWidget *parent, const QString &dataDir) : QMainWindow(parent)
//set up the user-interface
{
    //initial window size
    this->setMinimumHeight(300);
    this->setMinimumWidth(700);

    //menu, and two toolbars
    setup_menu_and_toolbar();

    //splitter for the tree and editor
    splitter = new QSplitter(this);

    //Tree and HTML editor windows are within the splitter
    //tree window on the left
    tree = new QTreeWidget ();
    tree->setColumnCount(1);
    tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect (tree, &QTreeWidget::itemClicked, this, &MainWindow::set_branch);
    //connect (tree, &QTreeWidget::itemActivated, this, &MainWindow::set_branch);
    connect (tree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::get_highlighted_branch);
    connect (tree, &QTreeWidget::itemChanged, this, &MainWindow::set_modified_flag);
    //the document name is shown in the window title, so the tree's own header
    //row is hidden; the category list starts right below the toolbar
    tree->setHeaderHidden(true);

    panelshortcut = new QShortcut (this);
    panelshortcut->setKey (Qt::CTRL | Qt::Key_Tab);
    connect (panelshortcut, &QShortcut::activated, this, &MainWindow::set_panel_focus);

    //HTML editor on right
    leafview = new QTextEdit (this);
    leafdoc = new QTextDocument(leafview);
    connect (leafdoc, &QTextDocument::contentsChanged, this, &MainWindow::get_data_from_leaf);

    splitter->addWidget(tree);
    splitter->addWidget(leafview);
    setCentralWidget(splitter);
    //re-fit images when the editor panel is resized (follows the window width)
    leafview->installEventFilter(this);

    //status bar on the bottom
    statustext = new QLabel (this);
    statustext->setObjectName("statustext");
    statustext->setText(tr("Status messages appear here"));
    statustext->setFrameStyle(QFrame::Plain);
    statustext->setAlignment(Qt::AlignBottom);
    statusBar()->addWidget(statustext);

    catflag = 1;

    //the data directory (images, help PDF, COPYING)
    DataDir.append(dataDir);
    //relative image links (bare filenames) resolve against this directory
    leafdoc->setBaseUrl (QUrl::fromLocalFile(DataDir + "/"));

    //help file to read
    Helpfilename.append (dataDir);
    Helpfilename.append ("/treeclehelp.pdf");

    //"COPYING" file to read
    Gnugplfilename.append (dataDir);
    Gnugplfilename.append ("/COPYING");

    //Currentfile name
    Currentfile.append("Noname.trc");

    //initial window size
    this->resize(990,630);

    //read the user's preferences
    readprefs();

    //the open/save dialogs start in the last used folder; on first run fall
    //back to the current working dir, then Documents, then home, then the data
    //dir (which is guaranteed to exist)
    if (Openpath.isEmpty()) {
        Openpath = QDir::currentPath();
        if (Openpath.isEmpty() || QDir(Openpath).exists() == false) {
            Openpath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            if (Openpath.isEmpty() || QDir(Openpath).exists() == false)
                Openpath = QDir::homePath();
            if (Openpath.isEmpty() || QDir(Openpath).exists() == false)
                Openpath = DataDir;
        }
    }

    //this->setFocus();

    //window title reflects the current file ("Treecle - Noname.trc" at start)
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    //release the per-file lock, if any (kept for a restored MainWindow stack
    //unwind; the QLockFile member is cleaned up here rather than left dangling)
    release_file_lock();
}

void MainWindow::quit()
{
    close();
}

