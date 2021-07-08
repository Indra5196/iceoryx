// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"

namespace iox
{
namespace popo
{
ServerPortUser::ServerPortUser(cxx::not_null<MemberType_t* const> serverPortDataPtr) noexcept
    : BasePort(serverPortDataPtr)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
    , m_chunkReceiver(&getMembers()->m_chunkReceiverData)

{
}

const ServerPortUser::MemberType_t* ServerPortUser::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ServerPortUser::MemberType_t* ServerPortUser::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}


cxx::expected<const mepoo::ChunkHeader*, ChunkReceiveResult> ServerPortUser::getRequest() noexcept
{
    return m_chunkReceiver.tryGet();
}

void ServerPortUser::releaseChunk(const mepoo::ChunkHeader* const requestHeader) noexcept
{
    /// @todo
    m_chunkReceiver.release(requestHeader);
}

bool ServerPortUser::hasNewRequests() const noexcept
{
    return !m_chunkReceiver.empty();
}

bool ServerPortUser::hasLostRequestsSinceLastCall() noexcept
{
    return m_chunkReceiver.hasLostChunks();
}

cxx::expected<mepoo::ChunkHeader*, AllocationError>
ServerPortUser::allocateResponse(const uint32_t userPayloadSize,
                                const uint32_t userPayloadAlignment,
                                const uint32_t userHeaderSize = 0U,
                                const uint32_t userHeaderAlignment = 1U) noexcept
{
    /// @todo
    auto result = m_chunkSender.tryAllocate(
        getUniqueID(), userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);

    // static_cast<ResponseHeader*>(result->userHeader())->setFireAndForget(getMembers()->m_fireAndForget);
    return result;
}


void ServerPortUser::freeResponse(mepoo::ChunkHeader* const responseHeader) noexcept
{
    /// @todo
    m_chunkSender.release(responseHeader);
}

void ServerPortUser::sendResponse(mepoo::ChunkHeader* const responseHeader, UniquePortId portId) noexcept
{
    /// @todo
    const auto offeringRequested = getMembers()->m_offeringRequested.load(std::memory_order_relaxed);

    if (offeringRequested)
    {
        m_chunkSender.sendToPort(responseHeader, portId);
    }
    else
    {
        // if the publisher port is not offered, we do not send the chunk but we put them in the history
        // this is needed e.g. for AUTOSAR Adaptive fields
        // just always calling send and relying that there are no subscribers if not offered does not work, as the list
        // of subscribers is updated asynchronously by RouDi (only RouDi has write access to the list of subscribers)
        m_chunkSender.pushToHistory(responseHeader);
    }
}

void ServerPortUser::offer() noexcept
{
    if (!getMembers()->m_offeringRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_offeringRequested.store(true, std::memory_order_relaxed);
    }
}

void ServerPortUser::stopOffer() noexcept
{
    if (getMembers()->m_offeringRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_offeringRequested.store(false, std::memory_order_relaxed);
    }
}

bool ServerPortUser::isOffered() const noexcept
{
    return getMembers()->m_offeringRequested.load(std::memory_order_relaxed);
}

bool ServerPortUser::hasClients() const noexcept
{
    return m_chunkSender.hasStoredQueues();
}

void ServerPortUser::setConditionVariable(ConditionVariableData& conditionVariableData,
                                          const uint64_t notificationIndex) noexcept
{
    m_chunkReceiver.setConditionVariable(conditionVariableData, notificationIndex);
}

void ServerPortUser::unsetConditionVariable() noexcept
{
    m_chunkReceiver.unsetConditionVariable();
}

bool ServerPortUser::isConditionVariableSet() const noexcept
{
    return m_chunkReceiver.isConditionVariableSet();
}

void ServerPortUser::releaseQueuedRequests() noexcept
{
    m_chunkReceiver.clear();
}

} // namespace popo
} // namespace iox
