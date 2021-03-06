// Copyright (c) Nuralogix. All rights reserved. Licensed under the MIT license.
// See LICENSE.txt in the project root for license information.

#ifndef DFXAPI_USERCOMMAND_HPP
#define DFXAPI_USERCOMMAND_HPP

#include <string>
#include <vector>

#include "dfx/api/cli/DFXAppCommand.hpp"

class UserGetCommand : public DFXAppCommand
{
public:
    UserGetCommand(CLI::App* get, std::shared_ptr<Options> options, DFXExitCode& result);
    DFXExitCode execute() override;

private:
    bool operateOnSelf;
};

#endif // DFXAPI_USERCOMMAND_HPP
