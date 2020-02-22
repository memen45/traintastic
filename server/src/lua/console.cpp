/**
 * server/src/lua/console.cpp - Lua console wrapper
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

#include "console.hpp"
#include "readonlytable.hpp"
#include "sandbox.hpp"
#include "script.hpp"
#include "../core/traintastic.hpp"

namespace Lua {

void Console::push(lua_State* L)
{
  lua_createtable(L, 0, 7);

  lua_pushcfunction(L, debug);
  lua_setfield(L, -2, "debug");
  lua_pushcfunction(L, info);
  lua_setfield(L, -2, "info");
  lua_pushcfunction(L, notice);
  lua_setfield(L, -2, "notice");
  lua_pushcfunction(L, warning);
  lua_setfield(L, -2, "warning");
  lua_pushcfunction(L, error);
  lua_setfield(L, -2, "error");
  lua_pushcfunction(L, critical);
  lua_setfield(L, -2, "critical");
  lua_pushcfunction(L, fatal);
  lua_setfield(L, -2, "fatal");

  ReadOnlyTable::setMetatable(L, -1);
}

void Console::log(lua_State* L, ::Console::Level level, const std::string& message)
{
  Traintastic::instance->console->log(level, Sandbox::getStateData(L).script().id, message);
}

int Console::log(lua_State* L, ::Console::Level level)
{
  const int top = lua_gettop(L);

  std::string message;
  for(int i = 1; i <= top; i++)
  {
    if(i != 1)
      message += " ";
    message += luaL_checkstring(L, i);
  }

  log(L, level, message);

  return 0;
}

}
