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

#ifndef IOX_POSH_POPO_TYPED_CLIENT_HPP
#define IOX_POSH_POPO_TYPED_CLIENT_HPP

#include "iceoryx_posh/popo/base_client.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"

namespace iox
{
namespace popo
{
///
/// @brief The ClientInterface class defines the client interface used by the Sample class to make it generic.
/// This allows any client specialization to be stored as a reference by the Sample class.
/// It is also needed to avoid circular dependencies between Sample and Client.
///
template <typename RequestType, typename H>
class PublisherInterface
{
  public:
    virtual void sendRequest(Sample<RequestType, H>&& sample) noexcept = 0;

    virtual ~PublisherInterface(){};

  protected:
    PublisherInterface() = default;
};
template <typename RequestType, typename ResponseType, typename H = iox::popo::RequestHeader, typename BaseClient_t = BaseClient<>>
class ClientImpl : public BaseClient_t, public PublisherInterface<RequestType, H>
{
    static_assert(!std::is_void<RequestType>::value, "The type `RequestType` must not be void. Use the UntypedClient for void types.");
    static_assert(!std::is_void<ResponseType>::value, "The type `ResponseType` must not be void. Use the UntypedClient for void types.");
    static_assert(!std::is_void<H>::value, "The user-header `H` must not be void.");

    static_assert(!std::is_const<RequestType>::value, "The type `RequestType` must not be const.");
    static_assert(!std::is_reference<RequestType>::value, "The type `RequestType` must not be a reference.");
    static_assert(!std::is_pointer<RequestType>::value, "The type `RequestType` must not be a pointer.");

    static_assert(!std::is_const<H>::value, "The user-header `H` must not be const.");
    static_assert(!std::is_reference<H>::value, "The user-header `H` must not be a reference.");
    static_assert(!std::is_pointer<H>::value, "The user-header must `H` not be a pointer.");

  public:
    ClientImpl(const capro::ServiceDescription& service,
                  const ClientOptions& clientOptions = ClientOptions());
    ClientImpl(const ClientImpl& other) = delete;
    ClientImpl& operator=(const ClientImpl&) = delete;
    ClientImpl(ClientImpl&& rhs) = default;
    ClientImpl& operator=(ClientImpl&& rhs) = default;
    virtual ~ClientImpl() = default;


    using ResponseHandlerType = std::function<void(ResponseType)>;
    ///
    /// @brief loan Get a sample from loaned shared memory and consctruct the data with the given arguments.
    /// @param args Arguments used to construct the data.
    /// @return An instance of the sample that resides in shared memory or an error if unable ot allocate memory to
    /// loan.
    /// @details The loaned sample is automatically released when it goes out of scope.
    ///
    template <typename... Args>
    cxx::expected<Sample<RequestType, H>, AllocationError> loanRequest(const uint32_t userPayloadSize = sizeof(RequestType),
                                                                      Args&&... args) noexcept;

    cxx::expected<Sample<const ResponseType, const iox::popo::ResponseHeader>, ChunkReceiveResult> takeResponses() noexcept;

    ///
    /// @brief publish Publishes the given sample and then releases its loan.
    /// @param sample The sample to publish.
    ///
    void sendRequest(Sample<RequestType, H>&& sample) noexcept override;

    void registerResponseHandler(ResponseHandlerType handler) noexcept;

    void unregisterResponseHandler() noexcept;

    void runResponseHandler(Sample<const ResponseType, const iox::popo::ResponseHeader>& sample) noexcept;

    ///
    /// @brief publishCopyOf Copy the provided value into a loaned shared memory chunk and publish it.
    /// @param val Value to copy.
    /// @return Error if unable to allocate memory to loan.
    ///
    // cxx::expected<AllocationError> publishCopyOf(const T& val) noexcept;
    ///
    /// @brief publishResultOf Loan a sample from memory, execute the provided callable to write to it, then publish it.
    /// @param c Callable with the signature void(T*, ArgTypes...) that write's it's result to T*.
    /// @param args The arguments of the callable.
    /// @return Error if unable to allocate memory to loan.
    ///
    // template <typename Callable, typename... ArgTypes>
    // cxx::expected<AllocationError> publishResultOf(Callable c, ArgTypes... args) noexcept;

  protected:
    using BaseClient_t::port;

  private:
    Sample<RequestType, H> convertRequestHeaderToSample(mepoo::ChunkHeader* const header) noexcept;

    cxx::expected<Sample<RequestType, H>, AllocationError> loanRequestSample(const uint32_t userPayloadSize) noexcept;

    using ClientSampleDeleter = SampleDeleter<typename BaseClient_t::PortType>;
    ClientSampleDeleter m_sampleDeleter{port()};

    ResponseHandlerType m_responseHandler{nullptr};
};

template <typename RequestType, typename ResponseType>
using Client = ClientImpl<RequestType, ResponseType>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/client.inl"

#endif // IOX_POSH_POPO_TYPED_CLIENT_HPP
