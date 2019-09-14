#pragma once

// Apoapse network protocol version number in the format [major][minor][minor][revision][revision] making one single integer with five digits. Example: 1.25.01 is 12501
static constexpr int protocolVersion = 1'01'00;
static constexpr UInt16 defaultServerPort = 3000;
static constexpr UInt16 defaultFileStreamPort = defaultServerPort + 1;
