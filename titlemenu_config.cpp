/********************************************************************
 K Win *- the KDE window manager                                       *
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

#include <QVBoxLayout>

#include <kwineffects.h>

#include "titlemenu_config.h"

using namespace KWin;

KWIN_EFFECT_CONFIG( titlemenu, TitleMenuConfig )

TitleMenuConfigForm::TitleMenuConfigForm(QWidget* parent): QWidget(parent)
{
	setupUi(this);
}

TitleMenuConfig::TitleMenuConfig(QWidget* parent, const QVariantList& args)
	: KCModule(EffectFactory::componentData(), parent, args)
{
	myUi = new TitleMenuConfigForm(this);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(myUi);
	connect(myUi->singleMenuCheckBox, SIGNAL(toggled(bool)), this, SLOT(changed()));
	connect(myUi->alwaysShowCheckBox, SIGNAL(toggled(bool)), this, SLOT(changed()));
}

TitleMenuConfig::~TitleMenuConfig()
{

}

void TitleMenuConfig::defaults()
{
	myUi->singleMenuCheckBox->setChecked(true);
	myUi->alwaysShowCheckBox->setChecked(true);
	emit changed(true);
}

void TitleMenuConfig::load()
{
// 	KCModule::load();
	KConfigGroup conf = EffectsHandler::effectConfig("TitleMenu");
	myUi->singleMenuCheckBox->setChecked(conf.readEntry("singlemenu", true));
	myUi->alwaysShowCheckBox->setChecked(conf.readEntry("alwaysshow", true));
	emit changed(false);
}

void TitleMenuConfig::save()
{
// 	KCModule::save();
	KConfigGroup conf = EffectsHandler::effectConfig("TitleMenu");
	conf.writeEntry("singlemenu", myUi->singleMenuCheckBox->isChecked());
	conf.writeEntry("alwaysshow", myUi->alwaysShowCheckBox->isChecked());
	conf.sync();
	emit changed(false);
	EffectsHandler::sendReloadMessage("titlemenu");
}

#include "titlemenu_config.moc"
