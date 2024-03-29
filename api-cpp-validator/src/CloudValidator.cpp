// Copyright (c) Nuralogix. All rights reserved. Licensed under the MIT license.
// See LICENSE.txt in the project root for license information.

#include "dfx/api/validator/CloudValidator.hpp"

#include "CloudValidatorMacros.hpp"

#include "fmt/format.h"

using namespace dfx::api;
using namespace dfx::api::validator;

CloudValidator::CloudValidator() : CloudAPI(CloudConfig()) {}

CloudStatus CloudValidator::connect(const CloudConfig& config)
{
    MACRO_RETURN_ERROR_IF_EMPTY(config.serverHost);
    return CloudStatus(CLOUD_OK);
}

const CloudValidator& CloudValidator::instance()
{
    static const CloudValidator instance;
    return instance;
}

const std::string& CloudValidator::getTransportType()
{
    static std::string INVALID_TRANSPORT_TYPE("Validator");
    return INVALID_TRANSPORT_TYPE;
}

CloudStatus CloudValidator::getServerStatus(CloudConfig& config, std::string& response)
{
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::login(CloudConfig& config)
{
    if (!config.authToken.empty()) {
        return CloudStatus(CLOUD_ALREADY_HAVE_USER_TOKEN, "Config already contains user token");
    }
    MACRO_RETURN_ERROR_IF_EMPTY(config.authEmail);
    MACRO_RETURN_ERROR_IF_EMPTY(config.authPassword);
    MACRO_RETURN_ERROR_IF_EMPTY(config.authOrg);
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::loginWithToken(CloudConfig& config, std::string& token)
{
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::logout(CloudConfig& config)
{
    MACRO_RETURN_ERROR_IF_EMPTY(config.authToken);
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::registerDevice(CloudConfig& config,
                                           const std::string& appName,
                                           const std::string& appVersion,
                                           const uint16_t tokenExpiresInSeconds,
                                           const std::string& tokenSubject)
{
    MACRO_RETURN_ERROR_IF_EMPTY(config.license);
    MACRO_RETURN_ERROR_IF_EMPTY(appName);
    MACRO_RETURN_ERROR_IF_EMPTY(appVersion);
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::unregisterDevice(CloudConfig& config)
{
    MACRO_RETURN_ERROR_IF_EMPTY(config.license);
    MACRO_RETURN_ERROR_IF_EMPTY(config.deviceID);
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::verifyToken(const CloudConfig& config, std::string& response)
{
    MACRO_RETURN_ERROR_IF_EMPTY(config.license);
    MACRO_RETURN_ERROR_IF_EMPTY(config.authOrg);
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::renewToken(const CloudConfig& config, std::string& token, std::string& refreshToken)
{
    return CloudStatus(CLOUD_OK);
}

CloudStatus CloudValidator::switchEffectiveOrganization(CloudConfig& config, const std::string& organizationID)
{
    return CloudStatus(CLOUD_OK);
}
