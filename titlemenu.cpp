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

#include <QTime>

#include <kwinconfig.h>
#include <kwinglutils.h>

#ifdef KWIN_HAVE_OPENGL_COMPOSITING
#include <GL/gl.h>
#endif
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#endif                                                                

#include <KColorScheme>
#include <KIcon>
#include <KConfigGroup>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kwindowsystem.h>

#include <kdebug.h>
#include <dbusmenuimporter.h>

#include <qjson/serializer.h>

#include <registrar.h>
#include "menucloner.h"
#include <QMenu>
#include <QMenuBar>
#include <Plasma/PushButton>
#include <QPainter>
#include <QLabel>

#include "menubar.h"

#include "titlemenu.h"
#include <QTimer>

class MyDBusMenuImporter : public DBusMenuImporter
{
public:
	MyDBusMenuImporter(const QString &service, const QString &path, GtkIconTable *table, QObject *parent)
	: DBusMenuImporter(service, path, parent)
	, mService(service)
	, mPath(path)
	, mGtkIconTable(table)
	{}
	
	QString service() const { return mService; }
	QString path() const { return mPath; }
	
protected:
	virtual QIcon iconForName(const QString &name_)
	{
		QString name;
		if (name_.startsWith("gtk")) {
			name = mGtkIconTable->value(name_);
	} else {
		name = name_;
	}
	return KIcon(name);
}

private:
	QString mService;
	QString mPath;
	GtkIconTable *mGtkIconTable;
	};

namespace KWin
{

KWIN_EFFECT(titlemenu, TitleMenuEffect)

TitleMenuEffect::TitleMenuEffect()
	: mGtkIconTable("/usr/share/icons/gnome/16x16") // FIXME Do not hardcode path
	, m_Registrar(new Registrar(this))
	, mMenuCloner(new MenuCloner(this))
	, mActiveWinId(0)
	, m_rootMenu(0)
	, m_menuBar(new MenuBar())
{
    m_mousePolling = false;
	m_moving = false;
	m_titlebar = QRect(0, 0, 0, 0);
	
	connect(m_Registrar, SIGNAL(WindowRegistered(WId, const QString&, const QDBusObjectPath&)),
			SLOT(slotWindowRegistered(WId, const QString&, const QDBusObjectPath&)));
	
	connect(m_Registrar, SIGNAL(WindowUnregistered(WId)),
			SLOT(slotWindowUnregistered(WId)));
	
	KWindowSystem* ws = KWindowSystem::self();
	connect(ws, SIGNAL(activeWindowChanged(WId)), SLOT(updateActiveWinId()));
	
	if (!m_Registrar->connectToBus()) {
		kWarning() << "Could not connect registrar to DBus";
		return;
	}
	
	reconfigure(ReconfigureAll);
	connect(effects,SIGNAL(windowFinishUserMovedResized(KWin::EffectWindow*)),
		this,SLOT(windowFinishUserMovedResized(KWin::EffectWindow*)));
	connect(effects,SIGNAL(windowStepUserMovedResized(KWin::EffectWindow*,const QRect&)),
		this,SLOT(windowStepUserMovedResized(KWin::EffectWindow*,const QRect&)));
	connect(effects,SIGNAL(windowMaximizedStateChanged(KWin::EffectWindow*,bool,bool)),
		this,SLOT(windowMaximizedStateChanged(KWin::EffectWindow*,bool,bool)));
	connect(effects,SIGNAL(windowActivated(KWIN::EffectWindow*)),
		this,SLOT(windowActivated(KWIN::EffectWindow*)));
	connect(effects,
		SIGNAL(mouseChanged(const QPoint&,const QPoint&,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)),
		this,
		SLOT(mouseChanged(const QPoint&,const QPoint&,Qt::MouseButtons,Qt::MouseButtons,Qt::KeyboardModifiers,Qt::KeyboardModifiers)));
	connect(effects,SIGNAL(windowAdded(KWin::EffectWindow*)),
		this,SLOT(windowAdded(KWin::EffectWindow*)));
	connect(effects,SIGNAL(windowMaximizedStateChanged(KWin::EffectWindow*,const QRect&)),
		this,SLOT(windowMaximizedStateChanged(KWin::EffectWindow*,const QRect&)));

}

TitleMenuEffect::~TitleMenuEffect()
{
    if (m_mousePolling)
        effects->stopMousePolling();
	
// 	foreach(MyDBusMenuImporter* i, mImporters.values()) {
// 		delete i;;
// 	}
// 	mImporters.clear();
	m_menuBar->deleteLater();;
// 	delete mMenuCloner;
	m_rootMenu->deleteLater();
// 	delete m_Registrar;
}

void TitleMenuEffect::reconfigure(ReconfigureFlags)
{
	if (!m_mousePolling) {
		effects->startMousePolling();
		m_mousePolling = true;
	}
	
	KConfigGroup conf = effects->effectConfig("TitleMenu");
	
	m_alwaysShow = conf.readEntry("alwaysshow", true);
	m_singleMenu = conf.readEntry("singlemenu", true);
	
	if(m_alwaysShow) {
		m_menuBar->show();
	} else {
		m_menuBar->hide();
	}
	
	if(m_rootMenu && m_menuBar) {
		m_menuBar->setSingleMenu(m_singleMenu);
		m_menuBar->setMenu(m_rootMenu);
	}
}

void TitleMenuEffect::mouseChanged(const QPoint& newM, const QPoint& /*oldM*/,
								   Qt::MouseButtons /*newB*/, Qt::MouseButtons /*oldB*/,
								   Qt::KeyboardModifiers /*newMod*/, Qt::KeyboardModifiers /*oldMod*/)
{
	if(m_alwaysShow) return;
	if(m_titlebar.contains(newM) && !m_moving && m_rootMenu) {
		m_menuBar->show();
	} else {
		m_menuBar->hide();
	}
}

void TitleMenuEffect::updateTitlebarPosition(EffectWindow* c)
{	
	if(m_alwaysShow) {
		m_menuBar->show();
	} else {
		m_menuBar->hide();
	}
	if(!m_moving) {
		int x = c->geometry().topLeft().x();
		int y = c->geometry().topLeft().y();
		int w = c->geometry().width();
		int h = c->geometry().height() - c->contentsRect().height();
		if(w > 0 && h > 0) {
			m_titlebar = QRect(x, y, w, h);
			updateMenuPosition();
		}
	}
// 	m_moving = false;
}

void TitleMenuEffect::updateTitlebarPosition(EffectWindow* c, const QRect &r)
{	
	if(m_alwaysShow) {
		m_menuBar->show();
	} else {
		m_menuBar->hide();
	}
	if(!m_moving) {
		int x = r.topLeft().x();
		int y = r.topLeft().y();
		int w = r.width();
		int h = r.height() - c->contentsRect().height();
		if(w > 0 && h > 0) {
			m_titlebar = QRect(x, y, w, h);
			updateMenuPosition();
		}
	}
// 	m_moving = false;
}


void TitleMenuEffect::windowActivated(EffectWindow* c)
{
// 	qDebug() << "windowActivated";
	if(!c) return;
	updateTitlebarPosition(c);
}

void TitleMenuEffect::windowMoveResizeGeometryUpdate(EffectWindow* c, const QRect& /*geometry*/)
{
// 	qDebug() << "windowMoveResizeGeometryUpdate";
	if(!c) return;
	updateTitlebarPosition(c);
}

void TitleMenuEffect::windowUserMovedResized(EffectWindow* c, bool first, bool last)
{
// 	qDebug() << "windowUserMovedResized" << first << last;
	if(!first && !last) {
		m_menuBar->hide();
		m_moving = true;
	} else if(last) {
		if(!c) return;
		m_moving = false;
		updateTitlebarPosition(c);
	}
}

void TitleMenuEffect::ActivateMenuItem(const QList< int >& )
{
}

QString TitleMenuEffect::DumpCurrentMenu()
{
	return DumpMenu(mActiveWinId);
}

void TitleMenuEffect::fillMap(QVariantMap* map, QList<QAction*> actions)
{
	QVariantList children;
	Q_FOREACH(QAction* action, actions) {
		QVariantMap child;
		if (action->isSeparator()) {
			child.insert("separator", true);
		} else {
			child.insert("label", action->text());
		}
		if (action->menu()) {
			fillMap(&child, action->menu()->actions());
		}
		children << child;
	}
	map->insert("submenu", children);
}

QVariant TitleMenuEffect::variantFromMenu(QMenu* menu)
{
	QVariantMap root;
	if (menu) {
		fillMap(&root, menu->actions());
	}
	return root;
}

QString TitleMenuEffect::DumpMenu(WId id)
{
	MyDBusMenuImporter* importer = mImporters.value(id);
	QVariant variant = variantFromMenu(importer ? importer->menu() : 0);
	QJson::Serializer serializer;
	return QString::fromUtf8(serializer.serialize(variant));
}

QString TitleMenuEffect::GetCurrentMenu(QDBusObjectPath& path) const
{
	QString service;
	MyDBusMenuImporter* importer = mImporters.value(mActiveWinId);
	if (importer) {
		service = importer->service();
		path = QDBusObjectPath(importer->path());
	} else {
		path = QDBusObjectPath("/");
	}
	return service;
}

void TitleMenuEffect::slotWindowRegistered(WId wid, const QString& service, const QDBusObjectPath& menuObjectPath)
{
// 	qDebug() << "Window registered" << wid;
	MyDBusMenuImporter* importer = new MyDBusMenuImporter(service, menuObjectPath.path(), &mGtkIconTable, this);
	delete mImporters.take(wid);
	mImporters.insert(wid, importer);
	connect(importer, SIGNAL(menuUpdated()), SLOT(slotMenuUpdated()));
	connect(importer, SIGNAL(actionActivationRequested(QAction*)), SLOT(slotActionActivationRequested(QAction*)));
	QMetaObject::invokeMethod(importer, "updateMenu", Qt::QueuedConnection);
}

void TitleMenuEffect::slotWindowUnregistered(WId wid)
{
// 	qDebug() << "Window unregistered" << wid;
	MyDBusMenuImporter* importer = mImporters.take(wid);
	if (importer) {
		importer->deleteLater();
	}
	if (wid == mActiveWinId) {
		mActiveWinId = 0;
	}
}

void TitleMenuEffect::updateMenuBar()
{
	WId winId = mActiveWinId;
	QMenu* menu = menuForWinId(winId);
	m_rootMenu = 0;
	
	if (!menu) {
		if (winId/* && !isDesktopWinId(winId)*/) {
			// We have an active window
			WId mainWinId = KWindowSystem::transientFor(winId);
			if (mainWinId) {
				// We have a parent window, use a disabled version of its
				// menubar if it has one.
				QMenu* mainMenu = menuForWinId(mainWinId);
				if (mainMenu) {
					mMenuCloner->setOriginalMenu(mainMenu);
					menu = mMenuCloner->clonedMenu();
				}
			}
// 			if (!menu) {
// 				// No suitable menubar but we have a window, use the
// 				// generic window menu
// 				mWindowMenuManager->setWinId(winId);
// 				menu = mWindowMenu;
// 			}
		}
	}
	//draw menu
	if(menu) {
// 		qDebug() << "We have a menu" << menu << winId;
		m_rootMenu = menu;
		m_moving = false;
		m_menuBar->setSingleMenu(m_singleMenu);
		m_menuBar->setMenu(m_rootMenu);
		QTimer::singleShot(250, this, SLOT(slotUpdateActiveTitlebar()));
	} else {
		m_menuBar->clearMenu();
	}
}

void TitleMenuEffect::slotUpdateActiveTitlebar()
{
// 	qDebug() << "slotUpdateActiveTitlebar()";
	updateTitlebarPosition(effects->activeWindow());
}

void TitleMenuEffect::updateMenuPosition()
{
	if(!m_rootMenu) return;
	m_menuBar->setMenuPosition(m_titlebar);
// 	effects->addRepaintFull();
}

void TitleMenuEffect::slotMenuUpdated()
{
	DBusMenuImporter* importer = static_cast<DBusMenuImporter*>(sender());
// 	qDebug() << "Menu updated" << importer << mImporters.value(mActiveWinId);
	
	if (mImporters.value(mActiveWinId) == importer) {
		updateMenuBar();
	}
}

void TitleMenuEffect::slotActionActivationRequested(QAction* action)
{
	Q_UNUSED(action)
	DBusMenuImporter* importer = static_cast<DBusMenuImporter*>(sender());
	
	if (mImporters.value(mActiveWinId) == importer) {
// 		qDebug() << action << "Clicked";
// 		if (!mMenuWidget) {
// 			kWarning() << "No mMenuWidget, should not happen!";
// 			return;
// 		}
// 		if (useButtonFormFactor()) {
// 			mMenuWidget->activateActionInMenu(action);
// 		} else {
// 			mMenuWidget->activateAction(action);
// 		}
	}
}

QMenu* TitleMenuEffect::menuForWinId(WId wid) const
{
	MyDBusMenuImporter* importer = mImporters.value(wid);
	return importer ? importer->menu() : 0;
}

void TitleMenuEffect::updateActiveWinId()
{
	WId id = KWindowSystem::activeWindow();
	if (id == mActiveWinId) {
		return;
	}
	mActiveWinId = id;
// 	qDebug() << "Window activated" << mActiveWinId;
	updateMenuBar();
}

void TitleMenuEffect::windowFinishUserMovedResized(EffectWindow *w)
{
//	qDebug() << "windowFinishUserMovedResized";
	updateTitlebarPosition(w);
}

void TitleMenuEffect::windowStepUserMovedResized(EffectWindow *w, const QRect &geometry)
{
//	qDebug() << "TitleMenuEffect::windowStepUserMovedResized";
	updateTitlebarPosition(w,geometry);
}

void TitleMenuEffect::windowMaximizedStateChanged(KWin::EffectWindow *w, bool horizontal, bool vertical)
{
	updateTitlebarPosition(w);
}

void TitleMenuEffect::windowAdded(KWin::EffectWindow *w)
{
	updateTitlebarPosition(w);
}

void TitleMenuEffect::windowGeometryShapeChanged(KWin::EffectWindow *w, const QRect &old)
{
	updateTitlebarPosition(w);
}

} // namespace
