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

#ifndef KWIN_TRACKMOUSE_H
#define KWIN_TRACKMOUSE_H

#include <QDBusObjectPath>
#include <QRect>
#include <QMenu>

#include <kwineffects.h>

#include "gtkicontable.h"

class MenuBar;
class QMenuBar;
class QAction;
class Registrar;
class MyDBusMenuImporter;
class MenuCloner;

namespace KWin
{

class TitleMenuEffect
    : public Effect
{
    Q_OBJECT
public:
    TitleMenuEffect();
    virtual ~TitleMenuEffect();
    virtual void mouseChanged(const QPoint& newM, const QPoint& /*oldM*/,
                              Qt::MouseButtons /*newB*/, Qt::MouseButtons /*oldB*/,
                              Qt::KeyboardModifiers /*newMod*/, Qt::KeyboardModifiers /*oldMod*/);
	virtual void windowActivated(EffectWindow *c);
	virtual void windowMoveResizeGeometryUpdate(EffectWindow *c, const QRect &/*geometry*/);
	virtual void windowUserMovedResized(EffectWindow *c, bool first, bool last);
	virtual void reconfigure(ReconfigureFlags);
	
private:
	typedef QHash<WId, MyDBusMenuImporter*> ImporterForWId;
    bool m_mousePolling, m_moving, m_alwaysShow, m_singleMenu;
	QRect m_titlebar;
	GtkIconTable mGtkIconTable;
	Registrar* m_Registrar;
	ImporterForWId mImporters;
	MenuCloner* mMenuCloner;
	WId mActiveWinId;
	QMenu* m_rootMenu;
	MenuBar* m_menuBar;
	
	
	QMenu* menuForWinId(WId) const;
	void updateMenuPosition();
	void updateTitlebarPosition(EffectWindow* c);
	void fillMap(QVariantMap* map, QList<QAction*> actions);
	QVariant variantFromMenu(QMenu* menu);
	
public Q_SLOTS:
	// DBus interface
	QString GetCurrentMenu(QDBusObjectPath&) const;
	void ActivateMenuItem(const QList<int>&);
	QString DumpCurrentMenu();
	QString DumpMenu(WId id);
	// /DBus interface
	
private Q_SLOTS:
	void slotUpdateActiveTitlebar();
	void updateMenuBar();
	void slotWindowRegistered(WId, const QString&, const QDBusObjectPath&);
	void slotWindowUnregistered(WId);
	void slotMenuUpdated();
	void slotActionActivationRequested(QAction* action);
	void updateActiveWinId();
	void windowFinishUserMovedResized(KWin::EffectWindow *w);
};

extern KWIN_EXPORT EffectsHandler* effects;

} // namespace

#endif
