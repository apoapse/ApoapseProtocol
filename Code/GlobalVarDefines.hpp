// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once

// Apoapse network protocol version number in the format [major][minor][minor][revision][revision] making one single integer with five digits. Example: 1.25.01 is 12501
static constexpr int protocolVersion = 1'01'01;
static constexpr int serverVersion = 1'00'00;
static constexpr UInt16 defaultServerPort = 3000;
static constexpr UInt16 defaultFileStreamPort = defaultServerPort + 1;
