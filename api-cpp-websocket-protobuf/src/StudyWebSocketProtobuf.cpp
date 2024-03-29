// Copyright (c) Nuralogix. All rights reserved. Licensed under the MIT license.
// See LICENSE.txt in the project root for license information.

#include "dfx/api/websocket/protobuf/StudyWebSocketProtobuf.hpp"
#include "dfx/api/validator/CloudValidator.hpp"
#include "dfx/api/web/WebServiceDetail.hpp"
#include "dfx/api/websocket/protobuf/CloudWebSocketProtobuf.hpp"

#include "nlohmann/json.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <sstream>
#include <string>

#include "libbase64.h"

#include "dfx/proto/studies.pb.h"

using namespace dfx::api;
using namespace dfx::api::websocket::protobuf;
using nlohmann::json;

StudyWebSocketProtobuf::StudyWebSocketProtobuf(const CloudConfig& config,
                                               std::shared_ptr<CloudWebSocketProtobuf> cloudWebSocketProtobuf)
    : cloudWebSocketProtobuf(std::move(cloudWebSocketProtobuf))
{
}

CloudStatus StudyWebSocketProtobuf::create(const CloudConfig& config,
                                           const std::string& name,
                                           const std::string& description,
                                           const std::string& templateID,
                                           const std::map<std::string, std::string>& studyConfig,
                                           std::string& studyID)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator, create(config, name, description, templateID, studyConfig, studyID));
    dfx::proto::studies::CreateRequest request;
    dfx::proto::studies::CreateResponse response;

    request.set_name(name);
    request.set_description(description);
    request.set_studytemplateid(templateID);
    auto studyconfig = request.config();
    studyconfig.insert(studyConfig.begin(), studyConfig.end());
    auto status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::Create, request, response);
    if (status.OK()) {
        studyID = response.id();
    }
    return status;
}

CloudStatus StudyWebSocketProtobuf::list(const CloudConfig& config,
                                         const std::unordered_map<StudyFilter, std::string>& filters,
                                         uint16_t offset,
                                         std::vector<Study>& studies,
                                         int16_t& totalCount)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator, list(config, filters, offset, studies, totalCount));

    dfx::proto::studies::ListRequest request;
    dfx::proto::studies::ListResponse response;
    auto status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::List, request, response);
    if (status.OK()) {
        const auto numberStudies = response.values_size();
        if (numberStudies == 0) {
            // It is possible with offset/limit to have no studies - but calling retrieveMultiple with an
            // empty set would return error so short-circuit here.
            return CloudStatus(CLOUD_OK);
        }

        std::vector<std::string> studyIDs;
        for (auto index = 0; index < response.values_size(); index++) {
            const auto& listResponseStudy = response.values(index);
            studyIDs.push_back(listResponseStudy.id());
        }

        return retrieveMultiple(config, studyIDs, studies);
    }

    return status;
}

CloudStatus StudyWebSocketProtobuf::retrieve(const CloudConfig& config, const std::string& studyID, Study& study)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator, retrieve(config, studyID, study));

    dfx::proto::studies::RetrieveRequest request;
    dfx::proto::studies::RetrieveResponse response;

    request.mutable_params()->set_id(studyID);

    auto status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::Retrieve, request, response);
    if (status.OK()) {
        study.id = response.id();
        study.name = response.name();
        study.description = response.description();
        study.templateID = response.studytemplateid();
        study.numberParticipants = response.participants();
        study.numberMeasurements = response.measurements();
        study.createdEpochSeconds = response.created();
        study.updatedEpochSeconds = response.updated();
        study.status = StudyStatusMapper::getEnum(response.statusid());
    }
    return status;
}

CloudStatus StudyWebSocketProtobuf::retrieveMultiple(const CloudConfig& config,
                                                     const std::vector<std::string>& studyIDs,
                                                     std::vector<Study>& studies)
{
    // Validate will occur by first retrieve call

    std::vector<Study> studyList;
    for (const auto& id : studyIDs) {
        Study study;
        auto status = retrieve(config, id, study);
        if (status.OK()) {
            studyList.push_back(study);
        } else {
            return status;
        }
    }

    // Copy all the items we retrieved - this ensures devices state consistent on failure
    // and allows client to pass existing items in list without us clearing.
    studies.insert(studies.end(), studyList.begin(), studyList.end());
    return CloudStatus(CLOUD_OK);
}

CloudStatus StudyWebSocketProtobuf::update(const CloudConfig& config,
                                           const std::string& studyID,
                                           const std::string& name,
                                           const std::string& description,
                                           StudyStatus status)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator, update(config, studyID, name, description, status));
    dfx::proto::studies::UpdateRequest request;
    dfx::proto::studies::UpdateResponse response;
    request.mutable_params()->set_id(studyID);
    request.set_name(name);
    request.set_description(description);
    request.set_statusid(StudyStatusMapper::getString(status));
    auto Status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::Update, request, response);
    return Status;
}

CloudStatus StudyWebSocketProtobuf::remove(const CloudConfig& config, const std::string& studyID)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator, remove(config, studyID));
    dfx::proto::studies::RemoveRequest request;
    dfx::proto::studies::RemoveResponse response;
    request.mutable_params()->set_id(studyID);
    auto Status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::Remove, request, response);
    return Status;
}

CloudStatus StudyWebSocketProtobuf::retrieveStudyConfig(const CloudConfig& config,
                                                        const std::string& studyID,
                                                        const std::string& sdkID,
                                                        const std::string& currentHashID,
                                                        std::vector<uint8_t>& studyData,
                                                        std::string& hashID)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator,
                              retrieveStudyConfig(config, studyID, sdkID, currentHashID, studyData, hashID));

    dfx::proto::studies::GetConfigRequest request;
    dfx::proto::studies::GetConfigResponse response;

    request.set_sdkid(sdkID);
    request.set_studyid(studyID);

    auto status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::GetConfig, request, response);
    if (status.OK()) {
        hashID = response.md5hash();

        // returned data is base64 encoded, client will need it as binary data
        const char* src = &(response.configfile()[0]);
        size_t srclen = response.configfile().size();
        std::vector<char> decodedData;
        decodedData.resize(srclen); // Needs to be at least 2/3 the length of input
        char* out = &(decodedData[0]);
        size_t outlen = 0;
        int flags = 0;
        int result = base64_decode(src, srclen, out, &outlen, flags);
        if (result == 1) {
            studyData.clear();
            studyData.reserve(outlen);
            for (int i = 0; i < outlen; i++) {
                studyData.push_back(out[i]);
            }
        } else {
            return CloudStatus(CLOUD_INTERNAL_ERROR, "Unable to decode study data");
        }
    }
    return status;
}

CloudStatus StudyWebSocketProtobuf::retrieveStudyTypes(const CloudConfig& config,
                                                       const StudyStatus status,
                                                       std::list<StudyType>& studyTypes)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator, retrieveStudyTypes(config, status, studyTypes));

    dfx::proto::studies::TypesRequest request;
    dfx::proto::studies::TypesResponse response;
    auto Status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::Types, request, response);
    if (Status.OK()) {
        for (auto index = 0; index < response.values_size(); index++) {
            const auto& listResponse = response.values(index);

            StudyType type;
            type.id = listResponse.id();
            type.name = listResponse.name();
            type.description = listResponse.description();
            type.status = StudyStatusMapper::getEnum(listResponse.statusid());
            studyTypes.push_back(type);
        }
    }

    return Status;
}

CloudStatus StudyWebSocketProtobuf::listStudyTemplates(const CloudConfig& config,
                                                       const StudyStatus status,
                                                       const std::string& type,
                                                       std::list<StudyTemplate>& studyTemplates)
{
    DFX_CLOUD_VALIDATOR_MACRO(StudyValidator, listStudyTemplates(config, status, type, studyTemplates));

    dfx::proto::studies::TemplatesRequest request;
    dfx::proto::studies::TemplatesResponse response;
    auto Status = cloudWebSocketProtobuf->sendMessage(dfx::api::web::Studies::RetrieveTemplates, request, response);
    if (Status.OK()) {
        for (auto index = 0; index < response.values_size(); index++) {
            const auto& listResponse = response.values(index);

            StudyTemplate studyTemplate;
            studyTemplate.id = listResponse.id();
            studyTemplate.name = listResponse.name();
            studyTemplate.description = listResponse.description();
            studyTemplate.status = StudyStatusMapper::getEnum(listResponse.statusid());
            studyTemplate.studyTypeID = listResponse.studytypeid();
            studyTemplate.createdEpochSeconds = listResponse.created();
            for (auto signalIndex = 0; signalIndex < listResponse.signals_size(); signalIndex++) {
                const auto& signal = listResponse.signals(signalIndex);
                studyTemplate.signalIDS.push_back(signal);
            }
            studyTemplates.push_back(studyTemplate);
        }
    }

    return Status;
}
