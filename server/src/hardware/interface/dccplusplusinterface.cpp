/**
 * server/src/hardware/interface/dccplusplusinterface.cpp
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

#include "dccplusplusinterface.hpp"
#include "../input/list/inputlisttablemodel.hpp"
#include "../output/list/outputlisttablemodel.hpp"
#include "../protocol/dccplusplus/messages.hpp"
#include "../protocol/dccplusplus/iohandler/serialiohandler.hpp"
#include "../protocol/dccplusplus/iohandler/simulationiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/serialport.hpp"
#include "../../world/world.hpp"

constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Channel | OutputListColumn::Address;

DCCPlusPlusInterface::DCCPlusPlusInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 115200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , dccplusplus{this, "dccplusplus", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , inputs{this, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , outputs{this, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  dccplusplus.setValueInternal(std::make_shared<DCCPlusPlus::Settings>(*this, dccplusplus.name()));
  decoders.setValueInternal(std::make_shared<DecoderList>(*this, decoders.name()));
  inputs.setValueInternal(std::make_shared<InputList>(*this, inputs.name(), inputListColumns));
  outputs.setValueInternal(std::make_shared<OutputList>(*this, outputs.name(), outputListColumns));

  Attributes::addDisplayName(device, DisplayName::Serial::device);
  Attributes::addEnabled(device, !online);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addMinMax(baudrate, SerialPort::baudrateMin, SerialPort::baudrateMax);
  Attributes::addValues(baudrate, SerialPort::baudrateValues);
  m_interfaceItems.insertBefore(baudrate, notes);

  Attributes::addDisplayName(dccplusplus, DisplayName::Hardware::dccplusplus);
  m_interfaceItems.insertBefore(dccplusplus, notes);

  Attributes::addDisplayName(decoders, DisplayName::Hardware::decoders);
  m_interfaceItems.insertBefore(decoders, notes);

  Attributes::addDisplayName(inputs, DisplayName::Hardware::inputs);
  m_interfaceItems.insertBefore(inputs, notes);

  Attributes::addDisplayName(outputs, DisplayName::Hardware::outputs);
  m_interfaceItems.insertBefore(outputs, notes);
}

bool DCCPlusPlusInterface::addDecoder(Decoder& decoder)
{
  const bool success = DecoderController::addDecoder(decoder);
  if(success)
    decoders->addObject(decoder.shared_ptr<Decoder>());
  return success;
}

bool DCCPlusPlusInterface::removeDecoder(Decoder& decoder)
{
  const bool success = DecoderController::removeDecoder(decoder);
  if(success)
    decoders->removeObject(decoder.shared_ptr<Decoder>());
  return success;
}

void DCCPlusPlusInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

bool DCCPlusPlusInterface::addInput(Input& input)
{
  const bool success = InputController::addInput(input);
  if(success)
    inputs->addObject(input.shared_ptr<Input>());
  return success;
}

bool DCCPlusPlusInterface::removeInput(Input& input)
{
  const bool success = InputController::removeInput(input);
  if(success)
    inputs->removeObject(input.shared_ptr<Input>());
  return success;
}

std::pair<uint32_t, uint32_t> DCCPlusPlusInterface::outputAddressMinMax(uint32_t channel) const
{
  using namespace DCCPlusPlus;

  switch(channel)
  {
    case Kernel::OutputChannel::dccAccessory:
      return {Kernel::dccAccessoryAddressMin, Kernel::dccAccessoryAddressMax};

    case Kernel::OutputChannel::turnout:
    case Kernel::OutputChannel::output:
      return {Kernel::idMin, Kernel::idMax};
  }

  assert(false);
  return {0, 0};
}

bool DCCPlusPlusInterface::addOutput(Output& output)
{
  const bool success = OutputController::addOutput(output);
  if(success)
    outputs->addObject(output.shared_ptr<Output>());
  return success;
}

bool DCCPlusPlusInterface::removeOutput(Output& output)
{
  const bool success = OutputController::removeOutput(output);
  if(success)
    outputs->removeObject(output.shared_ptr<Output>());
  return success;
}

bool DCCPlusPlusInterface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(channel, static_cast<uint16_t>(address), value);
}

bool DCCPlusPlusInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = DCCPlusPlus::Kernel::create<DCCPlusPlus::SimulationIOHandler>(dccplusplus->config());
      }
      else
      {
        m_kernel = DCCPlusPlus::Kernel::create<DCCPlusPlus::SerialIOHandler>(dccplusplus->config(), device.value(), baudrate.value(), SerialFlowControl::None);
      }

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);

          const bool powerOn = contains(m_world.state.value(), WorldState::PowerOn);

          if(!powerOn)
            m_kernel->powerOff();

          if(contains(m_world.state.value(), WorldState::Run))
          {
            m_kernel->clearEmergencyStop();
            restoreDecoderSpeed();
          }
          else
            m_kernel->emergencyStop();

          if(powerOn)
            m_kernel->powerOn();
        });
      m_kernel->setOnPowerOnChanged(
        [this](bool powerOn)
        {
          if(powerOn && !contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOn();
          else if(!powerOn && contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOff();
        });
      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);
      m_kernel->start();

      m_dccplusplusPropertyChanged = dccplusplus->propertyChanged.connect(
        [this](BaseProperty& property)
        {
          if(&property != &dccplusplus->startupDelay)
            m_kernel->setConfig(dccplusplus->config());
        });

      Attributes::setEnabled({device, baudrate}, false);
    }
    catch(const LogMessageException& e)
    {
      status.setValueInternal(InterfaceStatus::Offline);
      Log::log(*this, e.message(), e.args());
      return false;
    }
  }
  else if(m_kernel && !value)
  {
    Attributes::setEnabled({device, baudrate}, true);

    m_dccplusplusPropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
  }
  return true;
}

void DCCPlusPlusInterface::addToWorld()
{
  Interface::addToWorld();

  m_world.decoderControllers->add(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.inputControllers->add(std::dynamic_pointer_cast<InputController>(shared_from_this()));
  m_world.outputControllers->add(std::dynamic_pointer_cast<OutputController>(shared_from_this()));
}

void DCCPlusPlusInterface::loaded()
{
  Interface::loaded();

  check();
}

void DCCPlusPlusInterface::destroying()
{
  for(const auto& decoder : *decoders)
  {
    assert(decoder->interface.value() == std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
    decoder->interface = nullptr;
  }

  for(const auto& input : *inputs)
  {
    assert(input->interface.value() == std::dynamic_pointer_cast<InputController>(shared_from_this()));
    input->interface = nullptr;
  }

  for(const auto& output : *outputs)
  {
    assert(output->interface.value() == std::dynamic_pointer_cast<OutputController>(shared_from_this()));
    output->interface = nullptr;
  }

  m_world.decoderControllers->remove(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.inputControllers->remove(std::dynamic_pointer_cast<InputController>(shared_from_this()));
  m_world.outputControllers->remove(std::dynamic_pointer_cast<OutputController>(shared_from_this()));

  Interface::destroying();
}

void DCCPlusPlusInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        m_kernel->powerOff();
        m_kernel->emergencyStop();
        break;

      case WorldEvent::PowerOn:
        m_kernel->powerOn();
        break;

      case WorldEvent::Stop:
        m_kernel->emergencyStop();
        break;

      case WorldEvent::Run:
        m_kernel->powerOn();
        m_kernel->clearEmergencyStop();
        restoreDecoderSpeed();
        break;

      default:
        break;
    }
  }
}

void DCCPlusPlusInterface::check() const
{
  for(const auto& decoder : *decoders)
    checkDecoder(*decoder);
}

void DCCPlusPlusInterface::checkDecoder(const Decoder& decoder) const
{
  if(decoder.protocol != DecoderProtocol::Auto && decoder.protocol != DecoderProtocol::DCC)
    Log::log(decoder, LogMessage::C2002_DCCPLUSPLUS_ONLY_SUPPORTS_THE_DCC_PROTOCOL);

  if(decoder.protocol == DecoderProtocol::DCC && decoder.address <= 127 && decoder.longAddress)
    Log::log(decoder, LogMessage::C2003_DCCPLUSPLUS_DOESNT_SUPPORT_DCC_LONG_ADDRESSES_BELOW_128);

  if(decoder.speedSteps != Decoder::speedStepsAuto && decoder.speedSteps != dccplusplus->speedSteps)
    Log::log(decoder, LogMessage::W2003_COMMAND_STATION_DOESNT_SUPPORT_X_SPEEDSTEPS_USING_X, decoder.speedSteps.value(), dccplusplus->speedSteps.value());

  for(const auto& function : *decoder.functions)
    if(function->number > DCCPlusPlus::Config::functionNumberMax)
    {
      Log::log(decoder, LogMessage::W2002_COMMAND_STATION_DOESNT_SUPPORT_FUNCTIONS_ABOVE_FX, DCCPlusPlus::Config::functionNumberMax);
      break;
    }
}

void DCCPlusPlusInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}
