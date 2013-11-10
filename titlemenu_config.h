/********************************************************************
 K Win *- the KDE window manager
 This file is part of the KDE project.
 
 Copyright (C) 2011 Jimi Smith <smithj002@gmail.com>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************/

#ifndef KWIN_TITLEMENU_CONFIG_H
#define KWIN_TITLEMENU_CONFIG_H

#include <kcmodule.h>

#include "ui_titlemenu_config.h"

class KActionCollection;

namespace KWin
{
	
	class TitleMenuConfigForm : public QWidget, public Ui::TitleMenuConfigForm
	{
		Q_OBJECT
	public:
		explicit TitleMenuConfigForm(QWidget* parent);
	};
	
	class TitleMenuConfig : public KCModule
	{
		Q_OBJECT
	public:
		explicit TitleMenuConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
		~TitleMenuConfig();
		
	public slots:
		void save();
		void load();
		void defaults();
		
	private:
		TitleMenuConfigForm* myUi;
	};
	
} // namespace

#endif 
