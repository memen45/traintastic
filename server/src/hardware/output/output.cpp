/**
 * server/src/hardware/output/output.cpp
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

#include "output.hpp"
#include "../../world/world.hpp"
#include "outputlisttablemodel.hpp"
#include "../../core/attributes.hpp"

Output::Output(const std::weak_ptr<World> world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  value{this, "value", TriState::Undefined, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](TriState _value)
    {
      valueChanged(_value);
    },
    [this](TriState& value) -> bool
    {
      return setValue(value);
    }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addValues(value, TriStateValues);
  m_interfaceItems.add(value);
}

void Output::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->outputs->addObject(shared_ptr<Output>());
}

void Output::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  name.setAttributeEnabled(editable);
}

void Output::updateValue(TriState _value)
{
  value.setValueInternal(_value);
  valueChanged(value);
}