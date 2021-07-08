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

#ifndef IOX_POSH_POPO_UNTYPED_SERVER_INL
#define IOX_POSH_POPO_UNTYPED_SERVER_INL

namespace iox
{
namespace popo
{

// These static methods are not part of this file. Should be in application
static uint32_t SquareMethod(uint32_t requestMsg){
    return requestMsg * requestMsg;
}

static uint32_t CubeMethod(uint32_t requestMsg){
    return requestMsg * requestMsg * requestMsg;
}

static void HandleRequests(const void* userPayload, uint32_t& retval)
{
    const uint32_t* req = static_cast<const uint32_t*>(userPayload);
    auto header = static_cast<const iox::popo::RequestHeader*>(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader());
    Methods methodId = static_cast<Methods>(header->getMethodId());


    // uint32_t response;

    //throw 1;

    switch (methodId)
    {
    case Methods::SQUARE:
        retval = SquareMethod(*req);
        break;

    case Methods::CUBE:
        retval = CubeMethod(*req);
        break;
    
    default:
        break;
    }
}

template <typename BaseServer_t>
inline UntypedServerImpl<BaseServer_t>::UntypedServerImpl(const capro::ServiceDescription& service,
                                                                      const ServerOptions& serverOptions)
    : BaseServer(service, serverOptions)
{
    // Should be part of application
    registerRequestHandler(HandleRequests);
}

template <typename BaseServer_t>
inline void UntypedServerImpl<BaseServer_t>::send(void* const userPayload, UniquePortId portId) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().sendResponse(chunkHeader, portId);
}

template <typename BaseServer_t>
inline cxx::expected<void*, AllocationError>
UntypedServerImpl<BaseServer_t>::loan(const uint32_t userPayloadSize,
                                            const uint32_t userPayloadAlignment,
                                            const uint32_t userHeaderSize,
                                            const uint32_t userHeaderAlignment) noexcept
{
    auto result = port().allocateResponse(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<void*>(result.value()->userPayload());
    }
}

template <typename BaseServer_t>
inline cxx::expected<const void*, ChunkReceiveResult> UntypedServerImpl<BaseServer_t>::take() noexcept
{
    auto result = BaseServer::takeRequests();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    return cxx::success<const void*>(result.value()->userPayload());
}

template <typename BaseServer_t>
inline void UntypedServerImpl<BaseServer_t>::release(const void* const userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().releaseChunk(chunkHeader);
}

template <typename BaseServer_t>
inline void UntypedServerImpl<BaseServer_t>::registerRequestHandler(RequestHandlerType handler) noexcept
{
    m_requestHandler = handler;
}

template <typename BaseServer_t>
inline void UntypedServerImpl<BaseServer_t>::unregisterRequestHandler() noexcept
{
    m_requestHandler = nullptr;
}

template <typename BaseServer_t>
inline void UntypedServerImpl<BaseServer_t>::runRequestHandler(const void* userPayload, uint32_t& retval)
{
    if (m_requestHandler) m_requestHandler(userPayload, retval);
}

template <typename BaseServer_t>
inline UntypedServerImpl<BaseServer_t>::~UntypedServerImpl() noexcept
{
    BaseServer_t::m_trigger.reset();
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_SERVER_INL
