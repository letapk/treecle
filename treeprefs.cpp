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

void MainWindow::writeprefs()
{
    QSettings settings("treecle", "Treecle");

    settings.setValue("pos", pos());//window position

    settings.setValue("size", size());//window size

    settings.setValue("treewidth", tree->width());

    settings.setValue("edwidth", leafview->width());
}

void MainWindow::readprefs()
{
QString s;
QList<int> integerList;

    QSettings settings("treecle", "Treecle");

    QPoint pos = settings.value("pos", QPoint(20, 20)).toPoint();

    QSize size = settings.value("size", QSize(990, 630)).toSize();

    s = settings.value("treewidth", QString("330")).toString();
    integerList.append(s.toInt());

    s = settings.value("edwidth", QString("630")).toString();
    integerList.append(s.toInt());

    resize(size);
    move(pos);

    splitter->setSizes (integerList);
}
