//
// Copyright © 2017 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "PacketVersionResolver.hpp"

#include <armnn/utility/IgnoreUnused.hpp>

namespace armnn
{

namespace profiling
{

bool PacketKey::operator<(const PacketKey& rhs) const
{
    bool result = true;
    if (m_FamilyId == rhs.m_FamilyId)
    {
            result = m_PacketId < rhs.m_PacketId;
    }
    else if (m_FamilyId > rhs.m_FamilyId)
    {
        result = false;
    }
    return result;
}

bool PacketKey::operator>(const PacketKey& rhs) const
{
    return rhs < *this;
}

bool PacketKey::operator<=(const PacketKey& rhs) const
{
    return !(*this > rhs);
}

bool PacketKey::operator>=(const PacketKey& rhs) const
{
    return !(*this < rhs);
}

bool PacketKey::operator==(const PacketKey& rhs) const
{
    return m_FamilyId == rhs.m_FamilyId && m_PacketId == rhs.m_PacketId;
}

bool PacketKey::operator!=(const PacketKey& rhs) const
{
    return !(*this == rhs);
}

Version PacketVersionResolver::ResolvePacketVersion(uint32_t familyId, uint32_t packetId) const
{
    const PacketKey packetKey(familyId, packetId);

    if( packetKey == ActivateTimeLinePacket )
    {
        return Version(1, 1, 0);
    }
    if( packetKey == DeactivateTimeLinePacket )
    {
        return Version(1, 1, 0);
    }

    return Version(1, 0, 0);
}

} // namespace profiling

} // namespace armnn
