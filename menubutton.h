/*
    <one line to give the program's name and a brief idea of what it does.>
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


#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <Plasma/PushButton>
#include <QPushButton>

class MenuButton : public Plasma::PushButton
{
	Q_OBJECT

public:
	enum { Type = UserType + 1024 };
    explicit MenuButton(QGraphicsWidget* parent = 0);
    virtual int type() const { return Type; }
	QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint) const;
};

#endif // MENUBUTTON_H
