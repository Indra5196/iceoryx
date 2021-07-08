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
#include "rpc_base.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"

iox::UniquePortId g_portId(iox::popo::InvalidId);

void IcePerfRPCBase::preLatencyPerfTestClient(const uint32_t payloadSizeInBytes) noexcept
{
    sendPerfTopic(payloadSizeInBytes, RunFlag::RUN, g_portId);
}

void IcePerfRPCBase::postLatencyPerfTestClient() noexcept
{
    // Wait for the last response
    receivePerfTopic(g_portId);
}

void IcePerfRPCBase::releaseServer() noexcept
{   
    sendPerfTopic(sizeof(PerfTopic), RunFlag::STOP, g_portId);
}

iox::units::Duration IcePerfRPCBase::latencyPerfTestClient(const uint64_t numRoundTrips) noexcept
{
    auto start = std::chrono::high_resolution_clock::now();

    // run the performance test
    for (auto i = 0U; i < numRoundTrips; ++i)
    {
        auto perfTopic = receivePerfTopic(g_portId);
        sendPerfTopic(perfTopic.payloadSize, RunFlag::RUN, g_portId);
    }

    auto finish = std::chrono::high_resolution_clock::now();

    constexpr uint64_t TRANSMISSIONS_PER_ROUNDTRIP{2U};
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
    auto latencyInNanoSeconds =
        (static_cast<uint64_t>(duration.count()) / (numRoundTrips * TRANSMISSIONS_PER_ROUNDTRIP));
    return iox::units::Duration::fromNanoseconds(latencyInNanoSeconds);
}

void IcePerfRPCBase::latencyPerfTestServer() noexcept
{
    while (true)
    {
        iox::UniquePortId portId(iox::popo::InvalidId);
        auto perfTopic = receivePerfTopic(portId);

        // stop replying when no more run
        if (perfTopic.runFlag == RunFlag::STOP)
        {
            break;
        }

        sendPerfTopic(perfTopic.payloadSize, RunFlag::RUN, portId);
    }
}
