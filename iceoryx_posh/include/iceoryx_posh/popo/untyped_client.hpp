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

#ifndef IOX_POSH_POPO_UNTYPED_CLIENT_HPP
#define IOX_POSH_POPO_UNTYPED_CLIENT_HPP

#include "iceoryx_posh/popo/base_client.hpp"
#include "iceoryx_posh/popo/sample.hpp"

namespace iox
{
namespace popo
{
template <typename BaseClient_t = BaseClient<>>
class UntypedClientImpl : public BaseClient_t
{
  public:
    UntypedClientImpl(const capro::ServiceDescription& service,
                         const ClientOptions& clientOptions = ClientOptions());
    UntypedClientImpl(const UntypedClientImpl& other) = delete;
    UntypedClientImpl& operator=(const UntypedClientImpl&) = delete;
    UntypedClientImpl(UntypedClientImpl&& rhs) = default;
    UntypedClientImpl& operator=(UntypedClientImpl&& rhs) = default;
    virtual ~UntypedClientImpl() noexcept;

    ///
    /// @brief Take the chunk from the top of the receive queue.
    /// @return The user-payload pointer of the chunk taken.
    /// @details No automatic cleanup of the associated chunk is performed
    ///          and must be manually done by calling `release`
    ///
    cxx::expected<const void*, ChunkReceiveResult> take() noexcept;

    ///
    /// @brief Get a chunk from loaned shared memory.
    /// @param usePayloadSize The expected user-payload size of the chunk.
    /// @param userPayloadAlignment The expected user-payload alignment of the chunk.
    /// @return A pointer to the user-payload of a chunk of memory with the requested size or
    ///         an AllocationError if no chunk could be loaned.
    /// @note An AllocationError occurs if no chunk is available in the shared memory.
    ///
    cxx::expected<void*, AllocationError>
    loan(const uint32_t userPayloadSize,
         const uint32_t userPayloadAlignment = iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT,
         const uint32_t userHeaderSize = sizeof(iox::popo::RequestHeader),
         const uint32_t userHeaderAlignment = alignof(iox::popo::RequestHeader)) noexcept;

    ///
    /// @brief Publish the provided memory chunk.
    /// @param userPayload Pointer to the user-payload of the allocated shared memory chunk.
    /// @return Error if provided pointer is not a user-payload of a valid memory chunk.
    ///
    void send(void* const userPayload) noexcept;

    ///
    /// @brief Releases the ownership of the chunk provided by the user-payload pointer.
    /// @param userPayload pointer to the user-payload of the chunk to be released
    /// @details The userPayload pointer must have been previously provided by loan
    ///          and not have been already released. The chunk must not be accessed afterwards
    ///          as its memory may have been reclaimed.
    ///
    void release(const void* const userPayload) noexcept;

  protected:
    using BaseClient_t::port;
};

using UntypedClient = UntypedClientImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_client.inl"

#endif // IOX_POSH_POPO_UNTYPED_CLIENT_HPP
