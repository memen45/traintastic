/**
 * server/src/world/worldloader.cpp
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

#include "worldloader.hpp"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/string_generator.hpp>
#include "world.hpp"
#include "../utils/string.hpp"

#include "../hardware/commandstation/commandstations.hpp"
#include "../hardware/controller/controllers.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../hardware/decoder/decoderfunction.hpp"
#include "../hardware/input/inputs.hpp"
#include "../vehicle/rail/railvehicles.hpp"
#ifndef DISABLE_LUA_SCRIPTING
  #include "../lua/script.hpp"
#endif

using nlohmann::json;

WorldLoader::WorldLoader(const std::filesystem::path& filename) :
  m_world{World::create()}
{
  std::ifstream file(filename);
  if(!file.is_open())
    throw std::runtime_error("can't open " + filename.string());

  json data = json::parse(file);;

  m_world->m_filename = filename;
  m_world->m_uuid = boost::uuids::string_generator()(std::string(data["uuid"]));
  m_world->name = data[m_world->name.name()];

  // create a list of all objects
  for(json object : data["objects"])
  {
    if(auto it = object.find("id"); it != object.end())
      m_objects.insert({it.value().get<std::string>(), {object, nullptr, false}});
    else
      throw std::runtime_error("id missing");
  }

  // then create all objects
  for(auto& it : m_objects)
    if(!it.second.object)
      createObject(it.second);

  // and load their data
  for(auto& it : m_objects)
    if(!it.second.loaded)
      loadObject(it.second);

  // and finally notify loading is completed
  for(auto& it : m_objects)
    it.second.object->loaded();
}

ObjectPtr WorldLoader::getObject(std::string_view id)
{
  std::vector<std::string> ids;
  boost::split(ids, id, [](char c){ return c == '.'; });
  auto itId = ids.cbegin();

  ObjectPtr obj;
  if(auto it = m_objects.find(*itId); it != m_objects.end())
  {
    if(!it->second.object)
      createObject(it->second);
    obj = it->second.object;
  }

  while(obj && ++itId != ids.cend())
  {
    AbstractProperty* property = obj->getProperty(*itId);
    if(property && property->type() == ValueType::Object)
      obj = property->toObject();
    else
      obj = nullptr;
  }

  return obj;
}

void WorldLoader::createObject(ObjectData& objectData)
{
  assert(!objectData.object);

  std::string_view classId = objectData.json["class_id"];
  std::string_view id = objectData.json["id"];

  if(startsWith(classId, CommandStations::classIdPrefix))
    objectData.object = CommandStations::create(m_world, classId, id);
  else if(startsWith(classId, Controllers::classIdPrefix))
    objectData.object = Controllers::create(m_world, classId, id);
  else if(classId == Decoder::classId)
    objectData.object = Decoder::create(m_world, id);
  else if(classId == DecoderFunction::classId)
  {
    const std::string_view decoderId = objectData.json["decoder"];
    if(std::shared_ptr<Decoder> decoder = std::dynamic_pointer_cast<Decoder>(getObject(decoderId)))
      objectData.object = DecoderFunction::create(*decoder, id);
  }
  else if(startsWith(classId, Inputs::classIdPrefix))
    objectData.object = Inputs::create(m_world, classId, id);
  else if(startsWith(classId, RailVehicles::classIdPrefix))
    objectData.object = RailVehicles::create(m_world, classId, id);
#ifndef DISABLE_LUA_SCRIPTING
  else if(classId == Lua::Script::classId)
    objectData.object = Lua::Script::create(m_world, id);
#endif

  if(!objectData.object)
    {};//m_objects.insert(id, object);
}

void WorldLoader::loadObject(ObjectData& objectData)
{
  /*assert*/if(!objectData.object)return;
  assert(!objectData.loaded);
  loadObject(*objectData.object, objectData.json);
  objectData.loaded = true;
}

void WorldLoader::loadObject(Object& object, const json& data)
{
  if(AbstractObjectList* list = dynamic_cast<AbstractObjectList*>(&object))
  {
    json objects = data.value("objects", json::array());
    std::vector<ObjectPtr> items;
    items.reserve(objects.size());
    for(auto& [_, id] : objects.items())
      if(ObjectPtr item = getObject(id))
        items.emplace_back(std::move(item));
    list->setItems(items);
  }

  for(auto& [name, value] : data.items())
    if(AbstractProperty* property = object.getProperty(name))
      if(property->type() == ValueType::Object)
      {
        if(contains(property->flags(), PropertyFlags::SubObject))
        {
          loadObject(*property->toObject(), value);
        }
        else
        {
          if(value.is_string())
            property->load(getObject(value));
          else if(value.is_null())
            property->load(ObjectPtr());
        }
      }
      else
        property->load(value);

  //objectData.object->loaded();
}

/*
std::shared_ptr<Object> WorldLoader::getObject(std::string_view id)
{

}
*/
