// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_BASE_SERVER_INL
#define IOX_POSH_POPO_BASE_SERVER_INL

#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
template <typename port_t>
inline BaseServer<port_t>::BaseServer(const capro::ServiceDescription& service,
                                            const ServerOptions& serverOptions)
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareServer(service, serverOptions))
{
}

template <typename port_t>
inline BaseServer<port_t>::~BaseServer()
{
    m_port.destroy();
}

template <typename port_t>
inline uid_t BaseServer<port_t>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename port_t>
inline capro::ServiceDescription BaseServer<port_t>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename port_t>
inline void BaseServer<port_t>::offer() noexcept
{
    m_port.offer();
}

template <typename port_t>
inline void BaseServer<port_t>::stopOffer() noexcept
{
    m_port.stopOffer();
}

template <typename port_t>
inline bool BaseServer<port_t>::isOffered() const noexcept
{
    return m_port.isOffered();
}

template <typename port_t>
inline cxx::expected<const mepoo::ChunkHeader*, ChunkReceiveResult> BaseServer<port_t>::takeRequests() noexcept
{
    return m_port.getRequest();
}

template <typename port_t>
inline bool BaseServer<port_t>::hasRequests() const noexcept
{
    return m_port.hasRequest();
}

template <typename port_t>
inline bool BaseServer<port_t>::hasMissedRequests() const noexcept
{
    return m_port.hasMissedRequests();
}

template <typename port_t>
inline void BaseServer<port_t>::releaseQueuedRequests() noexcept
{
    m_port.releaseQueuedRequests();
}

template <typename port_t>
inline void BaseServer<port_t>::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        m_port.unsetConditionVariable();
        m_trigger.invalidate();
    }
}

template <typename port_t>
inline void BaseServer<port_t>::enableState(iox::popo::TriggerHandle&& triggerHandle,
                                                IOX_MAYBE_UNUSED const ServerState serverState) noexcept

{
    switch (serverState)
    {
    case ServerState::HAS_DATA:
        if (m_trigger)
        {
            LogWarn()
                << "The server is already attached with either the ServerState::HAS_DATA or "
                   "ServerEvent::DATA_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ServerState::HAS_DATA. Best practice is to call detach first.";

            errorHandler(
                Error::kPOPO__BASE_SERVER_OVERRIDING_WITH_STATE_SINCE_HAS_DATA_OR_DATA_RECEIVED_ALREADY_ATTACHED,
                nullptr,
                ErrorLevel::MODERATE);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}


template <typename port_t>
inline WaitSetIsConditionSatisfiedCallback
BaseServer<port_t>::getCallbackForIsStateConditionSatisfied(const ServerState serverState) const noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_DATA:
        return {*this, &SelfType::hasData};
    }
    return {};
}

template <typename port_t>
inline void BaseServer<port_t>::disableState(const ServerState serverState) noexcept
{
    switch (serverState)
    {
    case ServerState::HAS_DATA:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename port_t>
inline void BaseServer<port_t>::enableEvent(iox::popo::TriggerHandle&& triggerHandle,
                                                const ServerEvent serverEvent) noexcept
{
    switch (serverEvent)
    {
    case ServerEvent::DATA_RECEIVED:
        if (m_trigger)
        {
            LogWarn()
                << "The server is already attached with either the ServerState::HAS_DATA or "
                   "ServerEvent::DATA_RECEIVED to a WaitSet/Listener. Detaching it from previous one and "
                   "attaching it to the new one with ServerEvent::DATA_RECEIVED. Best practice is to call detach "
                   "first.";
            errorHandler(
                Error::kPOPO__BASE_SERVER_OVERRIDING_WITH_EVENT_SINCE_HAS_DATA_OR_DATA_RECEIVED_ALREADY_ATTACHED,
                nullptr,
                ErrorLevel::MODERATE);
        }
        m_trigger = std::move(triggerHandle);
        m_port.setConditionVariable(*m_trigger.getConditionVariableData(), m_trigger.getUniqueId());
        break;
    }
}

template <typename port_t>
inline void BaseServer<port_t>::disableEvent(const ServerEvent serverEvent) noexcept
{
    switch (serverEvent)
    {
    case ServerEvent::DATA_RECEIVED:
        m_trigger.reset();
        m_port.unsetConditionVariable();
        break;
    }
}

template <typename port_t>
const port_t& BaseServer<port_t>::port() const noexcept
{
    return m_port;
}

template <typename port_t>
port_t& BaseServer<port_t>::port() noexcept
{
    return m_port;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_SERVER_INL
