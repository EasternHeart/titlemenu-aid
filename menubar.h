/*
 *  A KWin effect to draw the application menu in the titlebar
 *  Copyright (C) 2011  Jimi Smith <email>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef MENUBAR_H
#define MENUBAR_H

#include <QGraphicsView>

class QGraphicsLinearLayout;
class QMenu;

class MenuBar : public QGraphicsView
{
	Q_OBJECT
public:
    MenuBar(QWidget* parent = 0);
	void setMenu(QMenu* menu);
	void setMenuPosition(QRect area);
	void clearMenu();
    virtual void drawBackground(QPainter* painter, const QRectF& rect);
	void setSingleMenu(bool m);
private:
	bool m_singleMenu;
	
	QGraphicsScene* m_scene;
	QGraphicsLinearLayout* m_layout;
    QGraphicsWidget* m_form;
private Q_SLOTS:
	void slotShowMenu();
};

#endif // MENUBAR_H
