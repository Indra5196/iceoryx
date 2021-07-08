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

#include "iceoryx_posh/internal/popo/ports/client_port_data.hpp"

namespace iox
{
namespace popo
{
ClientPortData::ClientPortData(const capro::ServiceDescription& serviceDescription,
                               const RuntimeName_t& runtimeName,
                               const ClientOptions& clientOptions,
                               mepoo::MemoryManager* const memoryManager,
                               const mepoo::MemoryInfo& memoryInfo) noexcept
    : BasePortData(serviceDescription, runtimeName, clientOptions.nodeName)
    , m_fireAndForget(clientOptions.fireAndForget)
    , m_connectRequested(clientOptions.connectOnCreate)
    , m_chunkSenderData(memoryManager, clientOptions.serverTooSlowPolicy, clientOptions.historyCapacity, memoryInfo)
    , m_chunkReceiverData(cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer, clientOptions.queueFullPolicy, memoryInfo)
{
    m_chunkReceiverData.ChunkQueueData_t::m_portId = BasePortData::m_uniqueId;
}

} // namespace popo
} // namespace iox
