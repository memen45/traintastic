/**
 * server/src/hardware/output/outputcontroller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUTCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUTCONTROLLER_HPP

#include <cstdint>
#include <unordered_map>
#include <memory>
#include "../../enum/tristate.hpp"

class Output;
class OutputKeyboard;

class OutputController
{
  public:
    using OutputMap = std::unordered_map<uint32_t, std::shared_ptr<Output>>;

  protected:
    OutputMap m_outputs;
    std::weak_ptr<OutputKeyboard> m_outputKeyboard;

  public:
    /**
     *
     */
    inline const OutputMap& outputs() const { return m_outputs; }

    /**
     *
     */
    virtual std::pair<uint32_t, uint32_t> outputAddressMinMax() const = 0;

    /**
     *
     */
    [[nodiscard]] virtual bool isOutputAddressAvailable(uint32_t address) const;

    /**
     * @brief Get the next unused output address
     *
     * @return An usused address or Output::invalidAddress if no unused address is available.
     */
    uint32_t getUnusedOutputAddress() const;

    /**
     *
     * @return \c true if changed, \c false otherwise.
     */
    [[nodiscard]] virtual bool changeOutputAddress(Output& output, uint32_t newAddress);

    /**
     *
     * @return \c true if added, \c false otherwise.
     */
    [[nodiscard]] virtual bool addOutput(Output& output);

    /**
     *
     * @return \c true if removed, \c false otherwise.
     */
    [[nodiscard]] virtual bool removeOutput(Output& output);

    /**
     * @brief ...
     */
    [[nodiscard]] virtual bool setOutputValue(uint32_t address, bool value) = 0;

    /**
     * @brief Update the output value
     *
     * This function should be called by the hardware layer whenever the output value changes.
     *
     * @param[in] address Output address
     * @param[in] value New output value
     */
    void updateOutputValue(uint32_t address, TriState value);

    /**
     *
     *
     */
    std::shared_ptr<OutputKeyboard> outputKeyboard();
};

#endif