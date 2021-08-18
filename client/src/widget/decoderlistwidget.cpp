/**
 * client/src/widget/decoderlistwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "decoderlistwidget.hpp"
#include "tablewidget.hpp"
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/abstractproperty.hpp"
#include "../mainwindow.hpp"

DecoderListWidget::DecoderListWidget(const ObjectPtr& object, QWidget* parent) :
  ObjectListWidget(object, parent)
{
  addActionEdit();
  addActionDelete();
}

void DecoderListWidget::objectDoubleClicked(const QString& id)
{
  if(m_object)
  {
    if(const auto& c = m_object->connection())
    {
      if(const auto& w = c->world())
      {
        if(auto* p = w->getProperty("edit"); p && !p->toBool())
        {
          MainWindow::instance->showObject(id, "", SubWindowType::Throttle);
        }
      }
    }
  }

  ObjectListWidget::objectDoubleClicked(id);
}
