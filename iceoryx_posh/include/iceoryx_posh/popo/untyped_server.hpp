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

#ifndef IOX_POSH_POPO_UNTYPED_SERVER_HPP
#define IOX_POSH_POPO_UNTYPED_SERVER_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/popo/base_server.hpp"

namespace iox
{
namespace popo
{
class Void
{
};

enum class Methods : uint64_t{
    SQUARE = 0x1,
    CUBE = 0x2
};

template <typename BaseServer_t = BaseServer<>>
class UntypedServerImpl : public BaseServer_t
{
  public:
    using BaseServer = BaseServer_t;
    using SelfType = UntypedServerImpl<BaseServer_t>;

    UntypedServerImpl(const capro::ServiceDescription& service,
                          const ServerOptions& serverOptions = ServerOptions());
    UntypedServerImpl(const UntypedServerImpl& other) = delete;
    UntypedServerImpl& operator=(const UntypedServerImpl&) = delete;
    UntypedServerImpl(UntypedServerImpl&& rhs) = default;
    UntypedServerImpl& operator=(UntypedServerImpl&& rhs) = default;
    virtual ~UntypedServerImpl() noexcept;

    ///
    /// @brief Take the chunk from the top of the receive queue.
    /// @return The user-payload pointer of the chunk taken.
    /// @details No automatic cleanup of the associated chunk is performed
    ///          and must be manually done by calling `release`
    ///
    cxx::expected<const void*, ChunkReceiveResult> take() noexcept;

    cxx::expected<void*, AllocationError>
    loan(const uint32_t userPayloadSize,
         const uint32_t userPayloadAlignment = iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT,
         const uint32_t userHeaderSize = sizeof(iox::popo::ResponseHeader),
         const uint32_t userHeaderAlignment = alignof(iox::popo::ResponseHeader)) noexcept;

    ///
    /// @brief Publish the provided memory chunk.
    /// @param userPayload Pointer to the user-payload of the allocated shared memory chunk.
    /// @return Error if provided pointer is not a user-payload of a valid memory chunk.
    ///
    void send(void* const userPayload, UniquePortId portId) noexcept;

    void runRequestHandler(const void* userPayload, uint32_t& retval);

    using RequestHandlerType = std::function<void(const void*, uint32_t& retval)>;

    void registerRequestHandler(RequestHandlerType handler) noexcept;

    void unregisterRequestHandler() noexcept;

    // static void RequestHandler(UntypedServerImpl* server);

    ///
    /// @brief Releases the ownership of the chunk provided by the user-payload pointer.
    /// @param userPayload pointer to the user-payload of the chunk to be released
    /// @details The userPayload pointer must have been previously provided by take and
    ///          not have been already released.
    ///          The chunk must not be accessed afterwards as its memory may have
    ///          been reclaimed.
    ///
    void release(const void* const userPayload) noexcept;

  protected:
    using BaseServer::port;

  private:
    RequestHandlerType m_requestHandler;
};

using UntypedServer = UntypedServerImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_server.inl"

#endif // IOX_POSH_POPO_UNTYPED_SERVER_HPP
