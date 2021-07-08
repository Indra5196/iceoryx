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

#include "iceoryx_client.hpp"

#include <chrono>
#include <thread>

IceoryxClient::IceoryxClient(const iox::capro::IdString_t& clientName) noexcept
    : m_client({"IcePerf", clientName, "C++-API"}, iox::popo::ClientOptions{1U})
{
}

void IceoryxClient::init() noexcept
{
    std::cout << "Waiting for connection" << std::flush;
    while (m_client.getConnectionState() != iox::ConnectionState::CONNECTED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << " [ success ]" << std::endl;
}

void IceoryxClient::shutdown() noexcept
{
    m_client.disconnect();

    std::cout << "Waiting for disconnection " << std::flush;
    while (m_client.getConnectionState() != iox::ConnectionState::NOT_CONNECTED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << " [ finished ]" << std::endl;
}

void IceoryxClient::sendPerfTopic(const uint32_t payloadSizeInBytes, const RunFlag runFlag, iox::UniquePortId& portId) noexcept
{
    (void)portId;

    m_client.loanRequest(payloadSizeInBytes).and_then([&](auto& userPayload) {
        // auto sendSample = static_cast<PerfTopic*>(userPayload);
        userPayload->payloadSize = payloadSizeInBytes;
        userPayload->runFlag = runFlag;
        userPayload->subPackets = 1;

        m_client.sendRequest(std::move(userPayload));
    });
}

PerfTopic IceoryxClient::receivePerfTopic(iox::UniquePortId& portId) noexcept
{
    (void)portId;
    bool hasReceivedSample{false};
    PerfTopic receivedSample;

    do
    {
        m_client.takeResponses().and_then([&](auto& data) {
            receivedSample = *data;
            hasReceivedSample = true;
            // m_client.release(data);
        });
    } while (!hasReceivedSample);

    return receivedSample;
}
