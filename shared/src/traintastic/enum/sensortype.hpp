/**
 * shared/src/traintastic/enum/sensortype.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_SENSORTYPE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_SENSORTYPE_HPP

#include <cstdint>
#include "enum.hpp"

enum class SensorType : uint8_t
{
  OccupyDetector = 0,
  ReedSwitch = 1,
};

TRAINTASTIC_ENUM(SensorType, "sensor_type", 2,
{
  {SensorType::OccupyDetector, "occupy_detector"},
  {SensorType::ReedSwitch, "reed_switch"},
});

#endif
