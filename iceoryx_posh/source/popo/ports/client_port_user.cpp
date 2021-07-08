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

#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"

namespace iox
{
namespace popo
{
ClientPortUser::ClientPortUser(cxx::not_null<MemberType_t* const> clientPortDataPtr) noexcept
    : BasePort(clientPortDataPtr)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
    , m_chunkReceiver(&getMembers()->m_chunkReceiverData)

{
}

const ClientPortUser::MemberType_t* ClientPortUser::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ClientPortUser::MemberType_t* ClientPortUser::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

cxx::expected<mepoo::ChunkHeader*, AllocationError>
ClientPortUser::allocateRequest(const uint32_t userPayloadSize,
                                const uint32_t userPayloadAlignment,
                                const uint32_t userHeaderSize,
                                const uint32_t userHeaderAlignment) noexcept
{
    auto result = m_chunkSender.tryAllocate(
        getUniqueID(), userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);

    // new (result.value()->userHeader()) RequestHeader();

    static_cast<RequestHeader*>(result.value()->userHeader())->setFireAndForget(getMembers()->m_fireAndForget);
    return result;
}

void ClientPortUser::freeRequest(mepoo::ChunkHeader* const requestHeader) noexcept
{
    /// @todo
    m_chunkSender.release(requestHeader);
}

void ClientPortUser::sendRequest(mepoo::ChunkHeader* const requestHeader) noexcept
{
    /// @todo
    const auto connectRequested = getMembers()->m_connectRequested.load(std::memory_order_relaxed);
    static_cast<RequestHeader*>(requestHeader->userHeader())->setUniquePortId(getUniqueID());
    if (connectRequested)
    {
        m_chunkSender.send(requestHeader);
    }
    else
    {
        // if the publisher port is not offered, we do not send the chunk but we put them in the history
        // this is needed e.g. for AUTOSAR Adaptive fields
        // just always calling send and relying that there are no subscribers if not offered does not work, as the list
        // of subscribers is updated asynchronously by RouDi (only RouDi has write access to the list of subscribers)
        m_chunkSender.pushToHistory(requestHeader);
    }
}

void ClientPortUser::connect() noexcept
{
    /// @todo
    if (!getMembers()->m_connectRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_connectRequested.store(true, std::memory_order_relaxed);
    }
}

void ClientPortUser::disconnect() noexcept
{
    /// @todo
    if (getMembers()->m_connectRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_connectRequested.store(false, std::memory_order_relaxed);
    }
}

ConnectionState ClientPortUser::getConnectionState() const noexcept
{
    return getMembers()->m_connectionState;
}

cxx::expected<const mepoo::ChunkHeader*, ChunkReceiveResult> ClientPortUser::getResponse() noexcept
{
    /// @todo
    return m_chunkReceiver.tryGet();
}

void ClientPortUser::releaseChunk(const mepoo::ChunkHeader* const responseHeader) noexcept
{
    /// @todo
    m_chunkReceiver.release(responseHeader);
}

bool ClientPortUser::hasNewResponses() const noexcept
{
    return !m_chunkReceiver.empty();
}

bool ClientPortUser::hasLostResponsesSinceLastCall() noexcept
{
    return m_chunkReceiver.hasLostChunks();
}

void ClientPortUser::setConditionVariable(ConditionVariableData& conditionVariableData,
                                          const uint64_t notificationIndex) noexcept
{
    m_chunkReceiver.setConditionVariable(conditionVariableData, notificationIndex);
}

void ClientPortUser::unsetConditionVariable() noexcept
{
    m_chunkReceiver.unsetConditionVariable();
}

bool ClientPortUser::isConditionVariableSet() const noexcept
{
    return m_chunkReceiver.isConditionVariableSet();
}

void ClientPortUser::releaseQueuedResponses() noexcept
{
    m_chunkReceiver.clear();
}

} // namespace popo
} // namespace iox
