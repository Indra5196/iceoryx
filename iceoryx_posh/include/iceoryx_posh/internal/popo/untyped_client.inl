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

#ifndef IOX_POSH_POPO_UNTYPED_CLIENT_INL
#define IOX_POSH_POPO_UNTYPED_CLIENT_INL

namespace iox
{
namespace popo
{
template <typename BaseClient_t>
inline UntypedClientImpl<BaseClient_t>::UntypedClientImpl(const capro::ServiceDescription& service,
                                                                   const ClientOptions& clientOptions)
    : BaseClient_t(service, clientOptions)
{
}

template <typename BaseClient_t>
inline void UntypedClientImpl<BaseClient_t>::send(void* const userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().sendRequest(chunkHeader);
}

template <typename BaseClient_t>
inline cxx::expected<void*, AllocationError>
UntypedClientImpl<BaseClient_t>::loan(const uint32_t userPayloadSize,
                                            const uint32_t userPayloadAlignment,
                                            const uint32_t userHeaderSize,
                                            const uint32_t userHeaderAlignment) noexcept
{
    auto result = port().allocateRequest(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<void*>(result.value()->userPayload());
    }
}

template <typename BaseClient_t>
inline cxx::expected<const void*, ChunkReceiveResult> UntypedClientImpl<BaseClient_t>::take() noexcept
{
    auto result = BaseClient_t::takeResponses();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    return cxx::success<const void*>(result.value()->userPayload());
}

template <typename BaseClient_t>
inline void UntypedClientImpl<BaseClient_t>::release(const void* const userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().releaseChunk(chunkHeader);
}

template <typename BaseClient_t>
inline UntypedClientImpl<BaseClient_t>::~UntypedClientImpl() noexcept
{
    BaseClient_t::m_trigger.reset();
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_CLIENT_INL
