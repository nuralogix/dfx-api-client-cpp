// Copyright (c) Nuralogix. All rights reserved. Licensed under the MIT license.
// See LICENSE.txt in the project root for license information.

#include "dfx/api/rest/ProfileREST.hpp"
#include "dfx/api/rest/CloudREST.hpp"
#include "dfx/api/validator/CloudValidator.hpp"

#include "nlohmann/json.hpp"
#include <fmt/format.h>
#include <sstream>
#include <string>

using namespace dfx::api;
using namespace dfx::api::rest;
using nlohmann::json;

CloudStatus
ProfileREST::create(const CloudConfig& config, const std::string& name, const std::string& email, Profile& profile)
{
    DFX_CLOUD_VALIDATOR_MACRO(ProfileValidator, create(config, name, email, profile));

    json response, request = {{"Name", name}, {"Email", email}};
    auto status = CloudREST::performRESTCall(config, web::Profiles::Create, config.authToken, {}, request, response);
    if (status.OK()) {
        auto profileID = response["ID"].get<std::string>();
        return retrieve(config, profileID, profile);
    }
    return status;
}

CloudStatus ProfileREST::list(const CloudConfig& config,
                              const std::unordered_map<ProfileFilter, std::string>& filters,
                              uint16_t offset,
                              std::vector<Profile>& profiles,
                              int16_t& totalCount)
{
    DFX_CLOUD_VALIDATOR_MACRO(ProfileValidator, list(config, filters, offset, profiles, totalCount));

    totalCount = -1; // Return unknown -1, zero would be a literal zero

    // REST: https://dfxapiversion10.docs.apiary.io/#reference/0/profiles/list-profiles
    std::string accountID;
    std::stringstream urlQuery;
    for (auto& filter : filters) {
        switch (filter.first) {
            case ProfileFilter::ProfileName:
                urlQuery << "UserProfileName=" << filter.second << "&";
                break;
            case ProfileFilter::ProfileStatus:
                urlQuery << "Status=" << filter.second << "&";
                break;
            case ProfileFilter::AccountId:
                accountID = filter.second;
                break;
            default:
                return CloudStatus(CLOUD_PARAMETER_VALIDATION_ERROR, "Invalid filter given");
        }
    }
    urlQuery << "Offset=" << offset;

    json response, request;
    CloudStatus status(CLOUD_OK);
    if (accountID.empty()) {
        status = CloudREST::performRESTCall(
            config, web::Profiles::List, config.authToken, {}, urlQuery.str(), request, response);
    } else {
        status = CloudREST::performRESTCall(
            config, web::Profiles::ListByUser, config.authToken, {accountID}, urlQuery.str(), request, response);
    }
    if (status.OK()) {
        if (!response.empty()) {
            std::vector<Profile> profilesTemp = response;
            profiles.insert(profiles.end(), profilesTemp.begin(), profilesTemp.end());

            if (profilesTemp.size() > 0) {
                // First element assuming there is one will have a TotalCount field which makes for
                // a non-uniform JSON schema so custom decode here
                if (response[0].contains("TotalCount")) {
                    totalCount = response[0]["TotalCount"];
                }
            }
        }
    }
    return status;
}

CloudStatus ProfileREST::retrieve(const CloudConfig& config, const std::string& profileID, Profile& profile)
{
    DFX_CLOUD_VALIDATOR_MACRO(ProfileValidator, retrieve(config, profileID, profile));
    json response, request;
    auto status =
        CloudREST::performRESTCall(config, web::Profiles::Retrieve, config.authToken, {profileID}, request, response);
    if (status.OK()) {
        if (!response.is_array()) {
            profile = response; // Must be a map of one value
            profile.id = profileID;
        } else {
            // Try looking through the list of returned items by name and email. This part of the
            // spec does not seem to be fully documented. I had planned on using list() to obtain
            // but server gave me back a list when querying by empty ID so just rolling with it.
            bool notFound = true;
            for (auto record : response) {
                std::string email = record["Email"];
                std::string name = record["Name"];
                if ((email.compare(profile.email) == 0) && (name.compare(profile.name) == 0)) {
                    profile = record;
                    profile.id = profileID;
                    notFound = false;
                    break;
                }
            }
            if (notFound) {
                return CloudStatus(CLOUD_RECORD_NOT_FOUND);
            }
        }
    }

    return status;
}

CloudStatus ProfileREST::update(const CloudConfig& config, const Profile& profile)
{
    DFX_CLOUD_VALIDATOR_MACRO(ProfileValidator, update(config, profile));
    json response, request = {{"Name", profile.name},
                              {"Email", profile.email},
                              {"Status", ProfileStatusMapper::getString(profile.status)}};
    auto Status =
        CloudREST::performRESTCall(config, web::Profiles::Update, config.authToken, {profile.id}, request, response);

    return Status;
}

CloudStatus ProfileREST::remove(const CloudConfig& config, const std::string& profileID)
{
    DFX_CLOUD_VALIDATOR_MACRO(ProfileValidator, remove(config, profileID));

    json response, request;

    auto status =
        CloudREST::performRESTCall(config, web::Profiles::Remove, config.authToken, {profileID}, request, response);

    return status;
}