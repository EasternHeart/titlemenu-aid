/*
    A KWin effect to draw the application menu in the titlebar
    Copyright (C) 2011  Jimi Smith <email>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <QGraphicsLinearLayout>
#include <QDebug>
#include <QMenu>
#include <QtGui/QPainter>

#include <KLocalizedString>
#include <KColorScheme>

// #include <Plasma/ToolButton>
#include <Plasma/WindowEffects>

#include "menubutton.h"

#include "menubar.h"

MenuBar::MenuBar(QWidget* parent)
	: QGraphicsView(parent)
{
	setWindowFlags(Qt::X11BypassWindowManagerHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setFrameShape(QFrame::NoFrame);
	QPalette pal = palette();
	QColor c = KColorScheme(QPalette::Normal, KColorScheme::Selection).background().color();
	c.setAlphaF(0.5);
	pal.setColor(backgroundRole(), Qt::transparent);
// 	pal.setColor(backgroundRole(), c);
	setPalette(pal);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setTransformationAnchor(NoAnchor);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	
	m_scene = new QGraphicsScene(this);
	m_layout = new QGraphicsLinearLayout(Qt::Horizontal);
	m_form = new QGraphicsWidget;
	m_form->setLayout(m_layout);
	m_form->setContentsMargins(0, 0, 0, 0);
	m_layout->setContentsMargins(0, 0, 0, 0);
	m_form->setGeometry(0, 0, 500, 27);
	m_scene->addItem(m_form);
	m_form->setPos(QPoint(0, 0));
	setScene(m_scene);
	
	Plasma::WindowEffects::overrideShadow(winId(), true);
}

void MenuBar::setMenuPosition(QRect area)
{
	setGeometry(area.x(), area.y(), m_form->size().width(), area.height());
	setSceneRect(0, 0, m_form->size().width(), area.height());
	m_form->setSizePolicy(m_form->sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);
	m_form->setMaximumHeight(area.height());
	foreach(QGraphicsItem* i, m_scene->items()) {
		MenuButton* p = qgraphicsitem_cast<MenuButton*>(i);
		if(p) {
			p->setMaximumHeight(area.height());
		}
	}
// 	int spare = area.height() - 20;
// 	int top = spare / 2;
// 	int bottom = spare - top;
// 	m_form->setContentsMargins(0, top, 0, bottom);
}

void MenuBar::setMenu(QMenu* menu)
{
	foreach(QGraphicsItem* i, m_scene->items()) {
		MenuButton* p = qgraphicsitem_cast<MenuButton*>(i);
		if(p) {
			m_scene->removeItem(p);
			p->deleteLater();
		}
	}
	if(m_singleMenu) {
		QMenu* mainMenu = new QMenu;
		QAction* action = mainMenu->addAction(i18n("Menu"));
		if (menu->actions().count() == 1 && menu->actions().first()->menu()) {
			// If there is only one top-level item, use it directly
			// This is useful for window and desktop menus, but is also more
			// efficient for applications with only one item in their menubar
			menu = menu->actions().first()->menu();
		}
		action->setMenu(menu);
		MenuButton* b = new MenuButton();
		b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
		b->setMaximumHeight(20);
		b->setAction(action);
		connect(b, SIGNAL(clicked()), this, SLOT(slotShowMenu()));
		m_layout->addItem(b);
	} else {
		foreach(QAction* a, menu->actions()) {
			QMenu* m = a->menu();
			if(m) {
				MenuButton* b = new MenuButton();
				b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
				b->setMaximumHeight(20);
				b->setAction(a);
				connect(b, SIGNAL(clicked()), this, SLOT(slotShowMenu()));
				m_layout->addItem(b);
			}
		}
	}
}

void MenuBar::clearMenu()
{
	foreach(QGraphicsItem* i, m_scene->items()) {
		MenuButton* p = qgraphicsitem_cast<MenuButton*>(i);
		if(p) {
			m_scene->removeItem(p);
			p->deleteLater();
		}
	}
	setMenuPosition(QRect(0, 0, 0 ,0));
}

void MenuBar::setSingleMenu(bool m)
{
	m_singleMenu = m;
}

void MenuBar::slotShowMenu()
{
	MenuButton* t = qobject_cast<MenuButton*>(QObject::sender());
	if(!t) return;
	QMenu* m = t->action()->menu();	
	QPointF p = mapToGlobal(t->scenePos().toPoint());
	QPoint pop = QPoint(p.x(), p.y() + t->size().height());
	m->popup(pop);
}

void MenuBar::drawBackground(QPainter* painter, const QRectF& rect)
{
// 	QColor c = KColorScheme(QPalette::Normal, KColorScheme::Selection).background().color();
// 	c.setAlphaF(0.95);
// 	painter->setBrush(c);
// 	painter->setPen(Qt::transparent);
// 	painter->drawRoundedRect(m_form->rect(), 4, 4);
}

