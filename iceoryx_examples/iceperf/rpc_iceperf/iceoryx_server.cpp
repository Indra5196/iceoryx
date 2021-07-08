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

#include "iceoryx_server.hpp"

#include <chrono>
#include <thread>

IceoryxServer::IceoryxServer(const iox::capro::IdString_t& serverName) noexcept
    : m_server({"IcePerf", serverName, "C++-API"}, iox::popo::ServerOptions{1U})
{
}

void IceoryxServer::init() noexcept
{
    std::cout << "Waiting for offer" << std::flush;
    while (!m_server.isOffered())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << " [ success ]" << std::endl;
}

void IceoryxServer::shutdown() noexcept
{
    m_server.stopOffer();

    std::cout << "Waiting for stop offer " << std::flush;
    while (m_server.isOffered())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << " [ finished ]" << std::endl;
}

void IceoryxServer::sendPerfTopic(const uint32_t payloadSizeInBytes, const RunFlag runFlag, iox::UniquePortId& portId) noexcept
{
    m_server.loanResponse(payloadSizeInBytes).and_then([&](auto& userPayload) {
        // auto sendSample = static_cast<PerfTopic*>(userPayload);
        userPayload->payloadSize = payloadSizeInBytes;
        userPayload->runFlag = runFlag;
        userPayload->subPackets = 1;

        m_server.sendResponse(std::move(userPayload), portId);
    });
}

PerfTopic IceoryxServer::receivePerfTopic(iox::UniquePortId& portId) noexcept
{
    bool hasReceivedSample{false};
    PerfTopic receivedSample;

    do
    {
        m_server.takeRequests().and_then([&](auto& data) {
            portId = data.getUserHeader().getUniquePortId();
            receivedSample = *data;
            hasReceivedSample = true;
            // m_server.release(data);
        });
    } while (!hasReceivedSample);

    return receivedSample;
}
