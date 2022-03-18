// Copyright (c) Nuralogix. All rights reserved. Licensed under the MIT license.
// See LICENSE.txt in the project root for license information.

#pragma once
#ifndef DFX_API_CLOUD_DEVICE_VALIDATOR_H
#define DFX_API_CLOUD_DEVICE_VALIDATOR_H

#include "dfx/api/DeviceAPI.hpp"
#include "dfx/api/validator/CloudValidator.hpp"

namespace dfx::api::validator
{

class DeviceValidator : public DeviceAPI
{
public:
    DeviceValidator() = default;

    ~DeviceValidator() override = default;

    CloudStatus create(const CloudConfig& config,
                       const std::string& name,
                       DeviceType type,
                       const std::string& identifier,
                       const std::string& version,
                       Device& device) override;

    CloudStatus list(const CloudConfig& config,
                     const std::unordered_map<DeviceFilter, std::string>& filters,
                     uint16_t offset,
                     std::vector<Device>& deviceList,
                     int16_t& totalCount) override;

    CloudStatus retrieve(const CloudConfig& config, const std::string& deviceID, Device& device) override;

    CloudStatus retrieveMultiple(const CloudConfig& config,
                                 const std::vector<std::string>& deviceIDs,
                                 std::vector<Device>& devices) override;

    CloudStatus update(const CloudConfig& config, const Device& device) override;

    CloudStatus remove(const CloudConfig& config, const std::string& deviceID) override;

    static const DeviceValidator& instance();
};

} // namespace dfx::api::validator

#endif // DFX_API_CLOUD_DEVICE_VALIDATOR_H
