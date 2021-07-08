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

#ifndef IOX_POSH_POPO_TYPED_CLIENT_INL
#define IOX_POSH_POPO_TYPED_CLIENT_INL

#include <cstdint>

namespace iox
{
namespace popo
{
template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
inline ClientImpl<RequestType, ResponseType, H, BaseClient_t>::ClientImpl(const capro::ServiceDescription& service,
                                                           const ClientOptions& clientOptions)
    : BaseClient_t(service, clientOptions)
{
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
template <typename... Args>
inline cxx::expected<Sample<RequestType, H>, AllocationError> ClientImpl<RequestType, ResponseType, H, BaseClient_t>::loanRequest(const uint32_t userPayloadSize,
                                                                                                                                    Args&&... args) noexcept
{
    return std::move(loanRequestSample(userPayloadSize).and_then([&](auto& sample) { new (sample.get()) RequestType(std::forward<Args>(args)...); }));
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
inline cxx::expected<Sample<RequestType, H>, AllocationError> ClientImpl<RequestType, ResponseType, H, BaseClient_t>::loanRequestSample(const uint32_t userPayloadSize) noexcept
{
    static constexpr uint32_t USER_HEADER_SIZE{std::is_same<H, mepoo::NoUserHeader>::value ? 0U : sizeof(H)};

    auto result = port().allocateRequest(userPayloadSize, alignof(RequestType), USER_HEADER_SIZE, alignof(H));
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<Sample<RequestType, H>>(convertRequestHeaderToSample(result.value()));
    }
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
inline void ClientImpl<RequestType, ResponseType, H, BaseClient_t>::sendRequest(Sample<RequestType, H>&& sample) noexcept
{
    auto userPayload = sample.release(); // release the Samples ownership of the chunk before publishing
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().sendRequest(chunkHeader);
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
inline cxx::expected<Sample<const ResponseType, const iox::popo::ResponseHeader>, ChunkReceiveResult>
ClientImpl<RequestType, ResponseType, H, BaseClient_t>::takeResponses() noexcept
{
    auto result = BaseClient_t::takeResponses();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    auto userPayloadPtr = static_cast<const ResponseType*>(result.value()->userPayload());
    auto samplePtr = cxx::unique_ptr<const ResponseType>(userPayloadPtr, m_sampleDeleter);
    return cxx::success<Sample<const ResponseType, const iox::popo::ResponseHeader>>(std::move(samplePtr));
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
inline Sample<RequestType, H>
ClientImpl<RequestType, ResponseType, H, BaseClient_t>::convertRequestHeaderToSample(mepoo::ChunkHeader* const header) noexcept
{
    return Sample<RequestType, H>(cxx::unique_ptr<RequestType>(reinterpret_cast<RequestType*>(header->userPayload()), m_sampleDeleter), *this);
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
void ClientImpl<RequestType, ResponseType, H, BaseClient_t>::registerResponseHandler(ResponseHandlerType handler) noexcept
{
    m_responseHandler = handler;
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
void ClientImpl<RequestType, ResponseType, H, BaseClient_t>::unregisterResponseHandler() noexcept
{
    m_responseHandler = nullptr;
}

template <typename RequestType, typename ResponseType, typename H, typename BaseClient_t>
void ClientImpl<RequestType, ResponseType, H, BaseClient_t>::runResponseHandler(Sample<const ResponseType, const iox::popo::ResponseHeader>& sample) noexcept
{
    if (sample.getUserHeader().hasServerError()) {
        std::cout << "Server Error Occured\n";
    }
    else
        if (m_responseHandler) m_responseHandler(*sample);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_CLIENT_INL
