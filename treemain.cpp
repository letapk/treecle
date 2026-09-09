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

//Last modified Sept 30, 2024

#include "treecle.h"

#include <cstdlib>

#include <QLockFile>

QString userpath;

void check_qtdata_dir ();
bool acquire_lock(QLockFile *lock);

int main(int argc, char *argv[])
//start user-interface
{
bool ok = false;
QString lockfilename;

    Q_INIT_RESOURCE(treecle);
    QApplication app(argc, argv);

    QTranslator appTranslator;
    ok = appTranslator.load("treecle_" + QLocale::system().name(), qApp->applicationDirPath());
    if (ok == true)
        app.installTranslator(&appTranslator);

    QTranslator qtTranslator;
    ok = qtTranslator.load("qt_" + QLocale::system().name(), qApp->applicationDirPath());
    if (ok == true)
        app.installTranslator(&qtTranslator);

    //get the path to the user's home directory
    userpath.clear();
    const char *home = getenv ("HOME");
    if (home == nullptr) {
        QMessageBox::critical (nullptr, "Treecle", "Environment variable HOME is not set.\nCannot determine the user data directory.");
        return 1;
    }
    userpath.append (home);
    userpath.append("/.treecle");//home/{account-name}/.treecle

    lockfilename.append(userpath);
    lockfilename.append("/treelockfile.lck");

    QLockFile lockfile(lockfilename);
    ok = acquire_lock(&lockfile);
    if (ok == false)//another instance holds the lock, exit
        return 0;

    //check for the tdj data directory and create it if required
    check_qtdata_dir();

    MainWindow mainwindow;
    mainwindow.setWindowTitle(QObject::tr("Treecle"));
    mainwindow.show();

    return app.exec();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
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
    connect (tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(set_branch(QTreeWidgetItem*)));
    //connect (tree, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(set_branch(QTreeWidgetItem*)));
    connect (tree, SIGNAL(itemSelectionChanged()), this, SLOT(get_highlighted_branch()));
    connect (tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT (set_modified_flag()));
    tree->setHeaderLabel(tr("Filename"));

    panelshortcut = new QShortcut (this);
    panelshortcut->setKey (Qt::CTRL | Qt::Key_Tab);
    connect (panelshortcut, SIGNAL(activated()), this, SLOT (set_panel_focus()));

    //HTML editor on right
    leafview = new QTextEdit (this);
    leafdoc = new QTextDocument(leafview);
    connect (leafdoc, SIGNAL(contentsChanged()), this, SLOT(get_data_from_leaf()));

    splitter->addWidget(tree);
    splitter->addWidget(leafview);
    setCentralWidget(splitter);

    //status bar on the bottom
    statustext = new QLabel (this);
    statustext->setText(tr("Status messages appear here"));
    statustext->setFrameStyle(QFrame::Plain);
    statustext->setAlignment(Qt::AlignBottom);
    statusBar()->addWidget(statustext);

    catflag = 1;

    //set the Homepath
    Homepath.append(userpath);
    //set the working directory to the data subdirectory
    QDir::setCurrent(Homepath);

    //help file to read
    Helpfilename.append (userpath);
    Helpfilename.append ("/treeclehelp.pdf");

    //"COPYING" file to read
    Gnugplfilename.append (userpath);
    Gnugplfilename.append ("/COPYING");

    //Currentfile name
    Currentfile.append("Noname.trc");

    //initial window size
    this->resize(990,630);

    //read the user's preferences
    readprefs();

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

