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

#ifndef IOX_POSH_POPO_SERVER_OPTIONS_HPP
#define IOX_POSH_POPO_SERVER_OPTIONS_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "port_queue_policies.hpp"
#include <cstdint>

namespace iox
{
namespace popo
{
/// @brief This struct is used to configure the client
struct ServerOptions
{
    /// @brief The size of the history chunk queue
    uint64_t queueCapacity{0U};

    /// @brief The size of the history chunk queue
    uint64_t historyCapacity{0U};

    /// @brief The max number of chunks received after response if chunks are available
    uint64_t historyRequest{0U};

    /// @brief The name of the node where the client should belong to
    iox::NodeName_t nodeName{""};

    /// @brief The option whether the client should already be connected when creating it
    bool offerOnCreate{true};

    /// @brief The option whether the client should block when the server queue is full
    QueueFullPolicy queueFullPolicy{QueueFullPolicy::BLOCK_PUBLISHER};

    /// @brief The option whether the client should block when the server queue is full
    SubscriberTooSlowPolicy clientTooSlowPolicy{SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA};
};

} // namespace popo
} // namespace iox
#endif // IOX_POSH_POPO_SERVER_OPTIONS_HPP
