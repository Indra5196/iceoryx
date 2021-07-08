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
#ifndef IOX_EXAMPLES_ICEPERF_ICEORYX_CLIENT_HPP
#define IOX_EXAMPLES_ICEPERF_ICEORYX_CLIENT_HPP

#include "rpc_base.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/popo/client.hpp"
// #include "iceoryx_posh/popo/untyped_subscriber.hpp"

class IceoryxClient : public IcePerfRPCBase
{
  public:
    IceoryxClient(const iox::capro::IdString_t& clientName) noexcept;
    void init() noexcept override;
    void shutdown() noexcept override;

  private:
    // void init() noexcept;
    void sendPerfTopic(const uint32_t payloadSizeInBytes, const RunFlag runFlag, iox::UniquePortId& portId) noexcept override;
    PerfTopic receivePerfTopic(iox::UniquePortId& portId) noexcept override;

    iox::popo::Client<PerfTopic, PerfTopic> m_client;
};

#endif // IOX_EXAMPLES_ICEPERF_ICEORYX_CLIENT_HPP
