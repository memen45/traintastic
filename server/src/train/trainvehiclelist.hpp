/**
 * server/src/train/trainvehiclelist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELIST_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELIST_HPP

#include "../core/objectlist.hpp"
#include "../vehicle/rail/railvehicle.hpp"

class Train;

class TrainVehicleList : public ObjectList<RailVehicle>
{
  private:
    inline Train& train();

  protected:
    bool isListedProperty(std::string_view name) final;
    void propertyChanged(BaseProperty& property) final;

  public:
    CLASS_ID("list.train_vehicle")

    Method<void(const std::shared_ptr<RailVehicle>&)> add;
    Method<void(const std::shared_ptr<RailVehicle>&)> remove;
    Method<void(uint32_t, uint32_t)> move;
    Method<void()> reverse;

    TrainVehicleList(Train& train_, std::string_view parentPropertyName);

    TableModelPtr getModel() final;
};

#endif
