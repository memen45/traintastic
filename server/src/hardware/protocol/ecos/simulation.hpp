/**
 * server/src/hardware/protocol/ecos/simulation.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_SIMULATION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_SIMULATION_HPP

#include <vector>
#include <cstdint>
#include "object/locomotiveprotocol.hpp"

namespace ECoS {

struct Simulation
{
  struct Object
  {
    uint16_t id;
  };

  struct Locomotive : Object
  {
    LocomotiveProtocol protocol;
    uint16_t address;
  };

  struct S88 : Object
  {
    uint8_t ports;
  };

  std::vector<Locomotive> locomotives;
  std::vector<S88> s88;

  void clear()
  {
    locomotives.clear();
    s88.clear();
  }
};

}

#endif
