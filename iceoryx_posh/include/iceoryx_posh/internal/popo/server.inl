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

#ifndef IOX_POSH_POPO_TYPED_SERVER_INL
#define IOX_POSH_POPO_TYPED_SERVER_INL

#include <cstdint>

namespace iox
{
namespace popo
{

static uint32_t SquareMethod(uint32_t requestMsg){
    return requestMsg * requestMsg;
}

static uint32_t CubeMethod(uint32_t requestMsg){
    return requestMsg * requestMsg * requestMsg;
}
template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
inline void ServerImpl<RequestType, ResponseType, H, BaseServer_t>::RequestHandler(ServerImpl<RequestType, ResponseType>* server){
    server->takeRequests().and_then([server](auto& sample) {
        bool fnf = sample.getUserHeader().getFireAndForget();
        iox::UniquePortId portId = sample.getUserHeader().getUniquePortId();
        bool serverError = false;
        ResponseType retval;
        try {
            retval = server->runRequestHandler(sample);
        }
        catch (...){
            serverError = true;
        }
        if (fnf)
            std::cout << "Fire and Forget: Sending No response\n";
        else {
            server->loanResponse(sizeof(ResponseType)).and_then([&](auto& sample) {
                *sample = retval;
                sample.getUserHeader().setServerError(serverError);
                std::cout << "Server Replying: " << retval << "\n";
                server->sendResponse(std::move(sample), portId);
            })
            .or_else([&](auto& error) {
                // Do something with the error
                std::cerr << "Unable to loan server sample, error code: " << static_cast<uint64_t>(error) << std::endl;
            });
        }
    });
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
inline ServerImpl<RequestType, ResponseType, H, BaseServer_t>::ServerImpl(const capro::ServiceDescription& service,
                                                           const ServerOptions& serverOptions)
    : BaseServer_t(service, serverOptions)
{
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
template <typename... Args>
inline cxx::expected<Sample<ResponseType, H>, AllocationError> ServerImpl<RequestType, ResponseType, H, BaseServer_t>::loanResponse(const uint32_t userPayloadSize,
                                                                                                                                    Args&&... args) noexcept
{
    return std::move(loanResponseSample(userPayloadSize).and_then([&](auto& sample) { new (sample.get()) ResponseType(std::forward<Args>(args)...); }));
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
inline cxx::expected<Sample<ResponseType, H>, AllocationError> ServerImpl<RequestType, ResponseType, H, BaseServer_t>::loanResponseSample(const uint32_t userPayloadSize) noexcept
{
    static constexpr uint32_t USER_HEADER_SIZE{std::is_same<H, mepoo::NoUserHeader>::value ? 0U : sizeof(H)};

    auto result = port().allocateResponse(userPayloadSize, alignof(ResponseType), USER_HEADER_SIZE, alignof(H));
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<Sample<ResponseType, H>>(convertResponseHeaderToSample(result.value()));
    }
}


template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
inline void ServerImpl<RequestType, ResponseType, H, BaseServer_t>::sendResponse(Sample<ResponseType, H>&& sample, UniquePortId portId) noexcept
{
    auto userPayload = sample.release(); // release the Samples ownership of the chunk before publishing
    auto response = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().sendResponse(response, portId);
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
inline cxx::expected<Sample<const RequestType, const iox::popo::RequestHeader>, ChunkReceiveResult>
ServerImpl<RequestType, ResponseType, H, BaseServer_t>::takeRequests() noexcept
{
    auto result = BaseServer_t::takeRequests();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    auto userPayloadPtr = static_cast<const RequestType*>(result.value()->userPayload());
    auto samplePtr = cxx::unique_ptr<const RequestType>(userPayloadPtr, m_sampleDeleter);
    return cxx::success<Sample<const RequestType, const iox::popo::RequestHeader>>(std::move(samplePtr));
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
inline Sample<ResponseType, H>
ServerImpl<RequestType, ResponseType, H, BaseServer_t>::convertResponseHeaderToSample(mepoo::ChunkHeader* const header) noexcept
{
    return Sample<ResponseType, H>(cxx::unique_ptr<ResponseType>(reinterpret_cast<ResponseType*>(header->userPayload()), m_sampleDeleter), *this);
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
void ServerImpl<RequestType, ResponseType, H, BaseServer_t>::registerRequestHandler(RequestHandlerType handler) noexcept
{
    m_requestHandler = handler;
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
void ServerImpl<RequestType, ResponseType, H, BaseServer_t>::unregisterRequestHandler() noexcept
{
    m_requestHandler = nullptr;
}

template <typename RequestType, typename ResponseType, typename H, typename BaseServer_t>
ResponseType ServerImpl<RequestType, ResponseType, H, BaseServer_t>::runRequestHandler(Sample<const RequestType, const iox::popo::RequestHeader>& sample)
{
    Methods methodId = static_cast<Methods>(sample.getUserHeader().getMethodId());
    ResponseType response;

    //throw 1;

    switch (methodId)
    {
    case Methods::SQUARE:
        response = SquareMethod(*sample);
        break;

    case Methods::CUBE:
        response = CubeMethod(*sample);
        break;
    
    default:
        break;
    }

    return response;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_SERVER_INL
