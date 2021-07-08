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

#ifndef IOX_POSH_POPO_BASE_SERVER_HPP
#define IOX_POSH_POPO_BASE_SERVER_HPP

#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/internal/popo/sample_deleter.hpp"
#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/enum_trigger_type.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_hoofs/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
using uid_t = UniquePortId;

enum class ServerEvent : EventEnumIdentifier
{
    DATA_RECEIVED
};

enum class ServerState : StateEnumIdentifier
{
    HAS_DATA
};

///
/// @brief The BaseServer class contains the common implementation for the different server specializations.
///
template <typename port_t = iox::ServerPortUserType>
class BaseServer
{
  public:
    using PortType = port_t;

    BaseServer(const BaseServer& other) = delete;
    BaseServer& operator=(const BaseServer&) = delete;
    BaseServer(BaseServer&& rhs) = delete;
    BaseServer& operator=(BaseServer&& rhs) = delete;
    virtual ~BaseServer();

    ///
    /// @brief uid Get the UID of the server.
    /// @return The server's UID.
    ///
    uid_t getUid() const noexcept;

    ///
    /// @brief getServiceDescription Get the service description of the server.
    /// @return The service description.
    ///
    capro::ServiceDescription getServiceDescription() const noexcept;

    ///
    /// @brief offer Offer the service to be subscribed to.
    ///
    void offer() noexcept;

    ///
    /// @brief stopOffer Stop offering the service.
    ///
    void stopOffer() noexcept;

    ///
    /// @brief isOffered
    /// @return True if service is currently being offered.
    ///
    bool isOffered() const noexcept;

    ///
    /// @brief hasServers
    /// @return True if currently has servers to the service.
    ///
    bool hasClients() const noexcept;

    bool hasRequests() const noexcept;

    cxx::expected<const mepoo::ChunkHeader*, ChunkReceiveResult> takeRequests() noexcept;

    bool hasMissedRequests() const noexcept;

    void releaseQueuedRequests() noexcept;

    friend class NotificationAttorney;

  protected:
    BaseServer() = default; // Required for testing.
    BaseServer(const capro::ServiceDescription& service, const ServerOptions& serverOptions);

    using SelfType = BaseServer<port_t>;

    void invalidateTrigger(const uint64_t trigger) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Attaches the triggerHandle to the internal trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] serverState the state which should be attached
    void enableState(iox::popo::TriggerHandle&& triggerHandle, const ServerState serverState) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Returns method pointer to the event corresponding
    /// hasTriggered method callback
    /// @param[in] serverState the state to which the hasTriggeredCallback is required
    WaitSetIsConditionSatisfiedCallback
    getCallbackForIsStateConditionSatisfied(const ServerState serverState) const noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Resets the internal triggerHandle
    /// @param[in] serverState the state which should be detached
    void disableState(const ServerState serverState) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Attaches the triggerHandle to the internal trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] serverEvent the event which should be attached
    void enableEvent(iox::popo::TriggerHandle&& triggerHandle, const ServerEvent serverState) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Resets the internal triggerHandle
    /// @param[in] serverEvent the event which should be detached
    void disableEvent(const ServerEvent serverEvent) noexcept;

    ///
    /// @brief port
    /// @return const accessor of the underlying port
    ///
    const port_t& port() const noexcept;

    ///
    /// @brief port
    /// @return accessor of the underlying port
    ///
    port_t& port() noexcept;

    port_t m_port{nullptr};

    TriggerHandle m_trigger;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/base_server.inl"

#endif // IOX_POSH_POPO_BASE_SERVER_HPP
