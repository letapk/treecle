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
#include <QDesktopServices>

#define FORWARD_ACTION(action1, action2) \
    connect(leafview->pageAction(action2), SIGNAL(changed()), SLOT(adjustActions()));

#define FOLLOW_CHECK(a1, a2) a1->setChecked(leafview->pageAction(a2)->isChecked())

QPixmap *pix;
QColor fontcolor;

extern QString userpath, Lockfilename;
extern void check_qtdata_dir ();

//Much of this is shamelessly copied and hacked from the examples within the Qt software
void MainWindow::setup_menu_and_toolbar ()
{
    //toolbar
    tb1 = new QToolBar (this);
    tb1->setGeometry(0, menuBar()->height(), menuBar()->width(), 30);
    tb1->setFloatable (false);
    tb1->setMovable(false);

    tb2 = new QToolBar (this);
    tb2->setGeometry(0, menuBar()->height() + tb1->height(), menuBar()->width(), 30);
    tb2->setFloatable (false);
    tb2->setMovable(false);

    //file menu
    filemenu = menuBar()->addMenu(tr("&File"));

    QIcon newIcon = QIcon::fromTheme("", QIcon(":/images/filenew.png"));
    QAction *newfile = new QAction(tr("&New file"), this);
    filemenu->addAction(newfile);
    newfile->setIcon(newIcon);
    newfile->setShortcut(QKeySequence::New);
    connect(newfile, SIGNAL(triggered()), this, SLOT(new_file()));

    QIcon openIcon = QIcon::fromTheme("", QIcon(":/images/fileopen.png"));
    QAction *openfile = new QAction(tr("&Open file"), this);
    filemenu->addAction(openfile);
    openfile->setIcon(openIcon);
    openfile->setShortcut(QKeySequence::Open);
    connect(openfile, SIGNAL(triggered()), this, SLOT(open_file()));

    QIcon saveIcon = QIcon::fromTheme("", QIcon(":/images/filesave.png"));
    QAction *savefile = new QAction(tr("&Save file"), this);
    filemenu->addAction(savefile);
    savefile->setIcon(saveIcon);
    savefile->setShortcut(QKeySequence::Save);
    connect(savefile, SIGNAL(triggered()), this, SLOT(save_file()));

    filemenu->addSeparator();

    QAction *quit = new QAction(tr("E&xit"), this);
    filemenu->addAction(quit);
    connect(quit, SIGNAL(triggered()), this, SLOT(quit()));

    //tree menu
    treemenu = menuBar()->addMenu(tr("&Tree"));

    QIcon addbrIcon = QIcon::fromTheme("", QIcon( ":/images/add-branch.png"));
    QAction *addbranch = new QAction(tr("&New Branch"), this);
    treemenu->addAction(addbranch);
    addbranch->setIcon(addbrIcon);
    connect(addbranch, SIGNAL(triggered()), this, SLOT(tree_addbranch()));

    QIcon addsubbrIcon = QIcon::fromTheme("", QIcon(":/images/add-subbranch.png"));
    QAction *addsubbranch = new QAction(tr("New &Sub-branch"), this);
    treemenu->addAction(addsubbranch);
    addsubbranch->setIcon(addsubbrIcon);
    connect(addsubbranch, SIGNAL(triggered()), this, SLOT(tree_addsubbranch()));

    treemenu->addSeparator();

    QIcon cutIcon = QIcon::fromTheme("", QIcon(":/images/editcut.png"));
    QAction *cutbranch = new QAction(tr("Cu&t branch"), this);
    treemenu->addAction(cutbranch);
    cutbranch->setIcon(cutIcon);
    connect(cutbranch, SIGNAL(triggered()), this, SLOT(tree_cutbranch()));

    QIcon copyIcon = QIcon::fromTheme("", QIcon(":/images/editcopy.png"));
    QAction *copybranch = new QAction(tr("&Copy branch"), this);
    treemenu->addAction(copybranch);
    copybranch->setIcon(copyIcon);
    connect(copybranch, SIGNAL(triggered()), this, SLOT(tree_copybranch()));

    QIcon pasteIcon = QIcon::fromTheme("", QIcon(":/images/editpaste.png"));
    QAction *pastebranch = new QAction(tr("&Paste branch"), this);
    treemenu->addAction(pastebranch);
    pastebranch->setIcon(pasteIcon);
    connect(pastebranch, SIGNAL(triggered()), this, SLOT(tree_pastebranch()));

    treemenu->addSeparator();

    QIcon expandIcon = QIcon::fromTheme("", QIcon(":/images/expandtree.png"));
    QAction *expand = new QAction(tr("&Expand all branches"), this);
    treemenu->addAction(expand);
    expand->setIcon(expandIcon);
    connect(expand, SIGNAL(triggered()), this, SLOT(expand_tree()));

    QIcon collapseIcon = QIcon::fromTheme("", QIcon(":/images/collapsetree.png"));
    QAction *collapse = new QAction(tr("Co&llapse all branches"), this);
    treemenu->addAction(collapse);
    collapse->setIcon(collapseIcon);
    connect(collapse, SIGNAL(triggered()), this, SLOT(collapse_tree()));

    treemenu->addSeparator();

    QIcon sortAscIcon = QIcon::fromTheme("", QIcon(":/images/view-sort-ascending.png"));
    QAction *sortAsc = new QAction(tr("&Sort tree in ascending order"), this);
    treemenu->addAction(sortAsc);
    sortAsc->setIcon(sortAscIcon);
    connect(sortAsc, SIGNAL(triggered()), this, SLOT(sort_asc_tree()));

    QIcon sortDescIcon = QIcon::fromTheme("", QIcon(":/images/view-sort-descending.png"));
    QAction *sortDesc = new QAction(tr("&Sort tree in descending order"), this);
    treemenu->addAction(sortDesc);
    sortDesc->setIcon(sortDescIcon);
    connect(sortDesc, SIGNAL(triggered()), this, SLOT(sort_desc_tree()));

    treemenu->addSeparator();

    QIcon delIcon = QIcon::fromTheme("", QIcon(":/images/editdelete.png"));
    QAction *delbranch = new QAction(tr("&Delete branch"), this);
    treemenu->addAction(delbranch);
    delbranch->setIcon(delIcon);
    connect(delbranch, SIGNAL(triggered()), this, SLOT(tree_delbranch()));

    //help menu
    helpmenu = menuBar()->addMenu(tr("&Help"));

    QAction *helpitem = new QAction(tr("&Help"), this);
    helpmenu->addAction(helpitem);
    connect(helpitem, SIGNAL(triggered()), this, SLOT(help()));

    QAction *aboutitem = new QAction(tr("&About"), this);
    helpmenu->addAction(aboutitem);
    connect(aboutitem, SIGNAL(triggered()), this, SLOT(about()));

    helpmenu->addSeparator();

    QAction *aboutQtitem = new QAction(tr("About &Qt"), this);
    helpmenu->addAction(aboutQtitem);
    connect(aboutQtitem, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actionTextBold = new QAction(QIcon::fromTheme("", QIcon(":/images/textbold.png")), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("", QIcon(":/images/textitalic.png")), tr("&Italic"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("", QIcon(":/images/textunder.png")), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    actionTextUnderline->setCheckable(true);

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction*)), this, SLOT(textAlign(QAction*)));

    actionAlignLeft = new QAction(QIcon::fromTheme("", QIcon(":/images/textleft.png")),tr("&Left"), grp);
    actionAlignCenter = new QAction(QIcon::fromTheme("",QIcon(":/images/textcenter.png")),tr("C&enter"), grp);
    actionAlignRight = new QAction(QIcon::fromTheme("",QIcon(":/images/textright.png")),tr("&Right"), grp);
    actionAlignJustify = new QAction(QIcon::fromTheme("",QIcon(":/images/textjustify.png")),tr("&Justify"), grp);

    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    QAction *actionInsertImage= new QAction(QIcon::fromTheme("", QIcon(":/images/insert-image.png")), tr("&Insert image"), this);
    actionInsertImage->setPriority(QAction::LowPriority);
    connect(actionInsertImage, SIGNAL(triggered()), this, SLOT(insertImage()));
    actionInsertImage->setCheckable(true);

    pix = new QPixmap (16, 16);
    fontcolor = Qt::black;
    pix->fill(fontcolor);
    QAction *actionTextColor = new QAction(*pix, tr("Text color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));

    comboFont = new QFontComboBox(tb1);
    QApplication::font().setPointSize(9);
    comboFont->setCurrentFont(QApplication::font());
    connect(comboFont, SIGNAL(currentFontChanged(QFont)), this, SLOT(fontFamily()));

    comboSize = new QComboBox(tb1);
    comboSize->setObjectName("comboSize");
    comboSize->setEditable(true);
    QFontDatabase db;
    foreach(int size, db.standardSizes())
        comboSize->addItem(QString::number(size));
    connect(comboSize, SIGNAL(activated(QString)), this, SLOT(textSize()));
    QApplication::font().setPointSize(6);
    comboSize->setCurrentIndex(comboSize->findText(QString::number(6)));

    srchbox = new QLineEdit (tb1);
    srchbox->setMaxLength(50);
    srchbox->setPlaceholderText("Enter search text...");
    srchbox->setMaximumWidth(200);
    srchbox->setMinimumWidth(200);

    QIcon srchnxtIcon = QIcon::fromTheme("", QIcon(":/images/go-down-search.png"));
    QAction *srchnxt = new QAction(tr("Search &next"), this);
    srchnxt->setIcon(srchnxtIcon);
    connect(srchnxt, SIGNAL(triggered()), this, SLOT(tree_srch_nxt()));

    QIcon srchpreIcon = QIcon::fromTheme("", QIcon(":/images/go-up-search.png"));
    QAction *srchpre = new QAction(tr("Search &previous"), this);
    srchpre->setIcon(srchpreIcon);
    connect(srchpre, SIGNAL(triggered()), this, SLOT(tree_srch_pre()));

    tb1->addAction(addbranch);
    tb1->addAction(addsubbranch);
    tb1->addSeparator();

    tb1->addAction(expand);
    tb1->addAction(collapse);
    tb1->addSeparator();

    tb1->addAction(sortAsc);
    tb1->addAction(sortDesc);
    tb1->addSeparator();

    tb1->addAction(cutbranch);
    tb1->addAction(copybranch);
    tb1->addAction(pastebranch);
    tb1->addSeparator();

    tb1->addAction(delbranch);
    tb1->addSeparator();

    tb1->addWidget(srchbox);
    tb1->addAction(srchnxt);
    tb1->addAction(srchpre);

    tb2->addAction(newfile);
    tb2->addAction(openfile);
    tb2->addAction(savefile);
    tb2->addSeparator();

    tb2->addAction(actionTextBold);
    tb2->addAction(actionTextItalic);
    tb2->addAction(actionTextUnderline);
    tb2->addSeparator();

    tb2->addActions(grp->actions());
    tb2->addSeparator();

    tb2->addAction(actionInsertImage);
    tb2->addSeparator();

    tb2->addAction(actionTextColor);
    tb2->addWidget(comboFont);
    tb2->addWidget(comboSize);

    QAction *selectall = new QAction(tr(""), this);
    connect(selectall, SIGNAL(triggered()), this, SLOT(select_all()));
}

void MainWindow::textBold()
{
    leafview->triggerPageAction(QWebPage::ToggleBold, true);
    FORWARD_ACTION(actionTextBold, QWebPage::ToggleBold);
}

void MainWindow::textItalic()
{
    leafview->triggerPageAction(QWebPage::ToggleItalic, true);
    FORWARD_ACTION(actionTextItalic, QWebPage::ToggleItalic);
}

void MainWindow::textUnderline()
{
    leafview->triggerPageAction(QWebPage::ToggleUnderline, true);
    FORWARD_ACTION(actionTextUnderline, QWebPage::ToggleUnderline);
}

void MainWindow::select_all()
{
    leafview->triggerPageAction(QWebPage::SelectAll, true);
    FORWARD_ACTION(selectall, QWebPage::SelectAll);
}

void MainWindow::adjustActions()
{
    FOLLOW_CHECK(actionTextBold, QWebPage::ToggleBold);
    FOLLOW_CHECK(actionTextItalic, QWebPage::ToggleItalic);
    FOLLOW_CHECK(actionTextUnderline, QWebPage::ToggleUnderline);
}

void MainWindow::textAlign(QAction *a)
{
    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty"));
        return;
    }

    QWebFrame *frame = leafview->page()->mainFrame();
    if (a == actionAlignLeft) {
        QString js = QString("document.execCommand(\"justifyLeft\", false, null)");
        frame->evaluateJavaScript(js);
    }
    else if (a == actionAlignCenter) {
        QString js = QString("document.execCommand(\"justifyCenter\", false, null)");
        frame->evaluateJavaScript(js);
    }
    else if (a == actionAlignRight) {
        QString js = QString("document.execCommand(\"justifyRight\", false, null)");
        frame->evaluateJavaScript(js);
    }
    else if (a == actionAlignJustify) {
        QString js = QString("document.execCommand(\"justifyFull\", false, null)");
        frame->evaluateJavaScript(js);
    }

}

void MainWindow::textColor()
{
QColor c;

    c = QColorDialog::getColor(fontcolor, this);
    if (c.isValid()) {
        QWebFrame *frame = leafview->page()->mainFrame();
        QString js = QString("document.execCommand(\"%1\", false, \"%2\")").arg("foreColor").arg(c.name());
        frame->evaluateJavaScript(js);
    }
    pix->fill(c);
    fontcolor = c;

}

void MainWindow::fontFamily()
{
QFont f;

    f = comboFont->currentFont();
    QWebFrame *frame = leafview->page()->mainFrame();
    QString js = QString("document.execCommand(\"%1\", false, \"%2\")").arg("fontName").arg(f.family());
    frame->evaluateJavaScript(js);

}

void MainWindow::textSize()
{
QString s;
float i;

    if (tree->topLevelItemCount() == 0)
        return;

    s = comboSize->currentText();

    //this is a kludge to get the fonts a reaonable size on my working monitor
    //it might not be optimal on other monitors
    i = s.toInt() / 2.0;

    QWebFrame *frame = leafview->page()->mainFrame();
    QString js = QString("document.execCommand(\"%1\", false, \"%2\")").arg("fontSize").arg(QString::number(i));
    frame->evaluateJavaScript(js);
}

void MainWindow::insertImage()
{
QString s, filters;

    if (tree->topLevelItemCount() == 0) {
        statustext->setText(tr("The tree is empty. Create a branch first."));
        return;
    }

    filters += tr("Common Graphics (*.png *.jpg *.jpeg *.gif);;");
    filters += tr("Portable Network Graphics (PNG) (*.png);;");
    filters += tr("JPEG (*.jpg *.jpeg);;");
    filters += tr("Graphics Interchange Format (*.gif);;");
    filters += tr("All Files (*)");

    QString file = QFileDialog::getOpenFileName(this, tr("Open image..."), QString(), filters);
    if (file.isEmpty())
        return;
    if (!QFile::exists(file))
        return;

    QUrl url = QUrl::fromLocalFile(file);
        QWebFrame *frame = leafview->page()->mainFrame();
        QString js = QString("document.execCommand(\"%1\", false, \"%2\")").arg("insertImage").arg(url.toString());
        frame->evaluateJavaScript(js);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
int i;

    if (tree->topLevelItemCount() > 0) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Treecle"), tr("Do you wish to save or discard the current tree?\n"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Cancel) {//user cancels
            statustext->setText(tr("The tree has not been saved "));
            event->ignore();
            return;
        }

        if (ret == QMessageBox::Save) {
            i = save_file();
            if (i == 1) {//file saved by the user
                statustext->setText(tr("Saved"));
            }
            else {//user cancelled the save operation
                statustext->setText(tr("The tree has not been saved"));
            }
            writeprefs();
            event->accept();
        }

        if (ret == QMessageBox::Discard) {
            statustext->setText(tr("Discarded"));
            writeprefs();
            event->accept();
        }
    }
    else {
        writeprefs();
        event->accept();
    }

}

void MainWindow::resizeEvent(QResizeEvent *event)
//move and shift the widgets when the window size changes
{
int w, h, sh, th;
QList<int> integerList;

    w = width();
    h = height();

    //sharing of width between tree and editor
    integerList.append(w/3);
    integerList.append(2*w/3);

    //toolbars
    th = menuBar()->height() + tb1->height();
    tb1->setGeometry(0, menuBar()->height(), w, 30);
    tb2->setGeometry(0, th, w, 30);

    //tree
    sh = menuBar()->height() + tb1->height() + tb2->height();
    splitter->setGeometry(5, sh, w-10, h-sh-30);

    //move the status text area
    statustext->setGeometry(10, h-30, w-20, 25);

    //pass the event up the chain
    QWidget::resizeEvent(event);
}

void MainWindow::help()
//open and show the user manual
{
bool ok;
QString s1;
QMessageBox msgBox;

    QFile file(Helpfilename);
    ok = file.exists();
    if (ok == false) {
        s1 = QObject::tr("The help file was not found\n");
        s1.append (QObject::tr("Please make sure that it is present in the hidden treecle data directory.\n"));
        s1.append (Helpfilename);
        s1.append("\n");
        s1.append (Homepath);
        s1.append("\n");
        s1.append (Lockfilename);


        msgBox.setText(s1);
        msgBox.exec();
    }
    else {
        QDesktopServices::openUrl (QUrl (Helpfilename));
    }
}
void MainWindow::about()
//open a window to show program information and copyright license
{
QFile file(Gnugplfilename);
QTextStream in(&file);
QTextBrowser *gnugpl;
bool ok;

    ok = file.open(QFile::ReadOnly);
    if (ok == false)
        return;

    gnugpl = new QTextBrowser ();
    gnugpl->setGeometry(100, 100, 800, 600);
    gnugpl->setWindowTitle (QObject::tr("About Treecle"));
    gnugpl->setPlainText(in.readAll());
    gnugpl->setAlignment(Qt::AlignLeft);
    gnugpl->show();
}

void check_qtdata_dir ()
{
QString qtpath, s1;
QDir qtdir;
QMessageBox msgBox;

    qtpath.append (userpath);
    qtdir = QDir (qtpath);

    if (qtdir.exists() == false) {
        s1.append (QObject::tr("The treecle data directory does not exist. "));
        s1.append (QObject::tr("This is required to store your work.\n"));
        s1.append (QObject::tr("Click OK to create a new, hidden subdirectory "));
        s1.append (QObject::tr("in your area with the name : \n"));
        s1.append (qtpath);
        s1.append ("\n");
        s1.append (QObject::tr("The help file treeclehelp.pdf should be copied\n"));
        s1.append (QObject::tr("in this location for online help to be available.\n"));
        s1.append (QObject::tr("The license file COPYING should also be copied\n"));
        s1.append (QObject::tr("in this location for the program details to be available.\n"));
        //s1.append (Lockfilename);

        msgBox.setText(s1);
        msgBox.exec();

        qtdir.mkdir(qtpath);
    }
}

void create_lockfile ()
{
    QFile file(Lockfilename);
    file.open(QFile::WriteOnly);
    file.close();
}

void delete_lockfile ()
{
    QFile file(Lockfilename);
    file.remove();
}

bool check_lockfile (void)
{
QMessageBox msgBox;
QString s1, s2, s3;
bool ok = false;

    //s1.append (Lockfilename);
    //msgBox.setText(s1);
    //msgBox.exec();

    QFile file(Lockfilename);
    ok = file.open(QFile::ReadOnly);
    if (ok == true) {//lockfile present, close it and inform user
        file.close();

        s1 = QObject::tr("It seems that \"Treecle\" is already running.");
        s2 = QObject::tr("If this is not the case, click \"Continue\", else click \"Abort\". ");
        s3 = QObject::tr("A lockfile has been found in the hidden treecle data-subdirectory. ");
        s3.append (QObject::tr("The program may be currently running in another window, in which case click \"Abort\". "));
        s3.append (QObject::tr("Alternatively, an earlier instance of the program may have failed to delete the lockfile. "));
        s3.append (QObject::tr("If you are sure that treecle is not running in this account, click \"Continue\". "));
        s3.append (QObject::tr("See the user manual about the risks of running two instances of the program at the same time.\n"));
        //s3.append (Lockfilename);

        msgBox.setText(s1);
        msgBox.setInformativeText(s2);
        msgBox.setDetailedText(s3);

        msgBox.addButton(QObject::tr("Continue"), QMessageBox::ApplyRole);
        msgBox.addButton(QObject::tr("Abort"), QMessageBox::RejectRole);

        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::ApplyRole://continue
                file.remove();
                return true;
                break;
            case QMessageBox::RejectRole://abort
                return false;
                break;
        }
    }

    return true;//lockfile absent
}
