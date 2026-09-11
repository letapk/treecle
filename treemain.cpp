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

#include "treecle.h"

#include <QLockFile>

#if !defined(TREECLE_TEST_BUILD)
int main(int argc, char *argv[])
//start user-interface
{
bool ok = false;
QString lockfilename;
QString dataDir;

    Q_INIT_RESOURCE(treecle);
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

    //create the data directory if it is still missing (a fresh install), so
    //that the lock file and help/license files have a place to live
    QDir().mkpath(dataDir);

    lockfilename.append(dataDir);
    lockfilename.append("/treelockfile.lck");

    QLockFile lockfile(lockfilename);
    ok = acquire_lock(&lockfile);
    if (ok == false)//another instance holds the lock, exit
        return 0;

    //check for the data directory and create it if required
    check_and_make_data_dir(dataDir);

    MainWindow mainwindow(nullptr, dataDir);
    mainwindow.setWindowTitle(QObject::tr("Treecle"));
    mainwindow.show();

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
    tree->setHeaderLabel(tr("Filename"));

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
}

MainWindow::~MainWindow()
{
    //the lockfile is released automatically when the QLockFile object in main() is destroyed
}

void MainWindow::quit()
{
    close();
}

