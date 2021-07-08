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

#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_handler.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

bool keepRunning = true;
constexpr char APP_NAME[] = "iox-cpp-callbacks-server-untyped";

iox::posix::Semaphore shutdownSemaphore =
    iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();

static void sigHandler(int f_sig IOX_MAYBE_UNUSED)
{
    shutdownSemaphore.post().or_else([](auto) {
        std::cerr << "unable to call post on shutdownSemaphore - semaphore corrupt?" << std::endl;
        std::exit(EXIT_FAILURE);
    });
    keepRunning = false;
}

void RequestHandler(iox::popo::UntypedServer* server) {
    server->take().and_then([server](const void* userPayload) {
        auto header = static_cast<const iox::popo::RequestHeader*>(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader());
        bool serverError = false;
        uint32_t retval;
        bool fnf = header->getFireAndForget();
        iox::UniquePortId portId = header->getUniquePortId();
        try {
            server->runRequestHandler(userPayload, retval);
        }
        catch (...){
            serverError = true;
        }

        server->release(userPayload);

        if (fnf)
            std::cout << "Fire and Forget: Sending No response\n";
        else {
        server->loan(sizeof(uint32_t)).and_then([&](void* userPayload) {
            uint32_t* res = static_cast<uint32_t*>(userPayload);
            *res = retval;
            auto header = static_cast<iox::popo::ResponseHeader*>(iox::mepoo::ChunkHeader::fromUserPayload(userPayload)->userHeader());
            header->setServerError(serverError);
            std::cout << "Server Replying: " << retval << "\n";
            server->send(userPayload, portId);
        })
        .or_else([&](auto& error) {
            // Do something with the error
            std::cerr << "Unable to loan server sample, error code: " << static_cast<uint64_t>(error) << std::endl;
        });
    }
    });
}

int main()
{
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    std::cout << "Runtime Initialized Server\n";
    iox::popo::Listener listener;
    std::cout << "Listener Created Server\n";
    iox::popo::UntypedServer myServerLeft({"Radar", "FrontLeft"});
    std::cout << "Server Created\n";
    // myServerLeft.registerRequestHandler(AdjustMethod);

    listener.attachEvent(myServerLeft, iox::popo::ServerEvent::DATA_RECEIVED, iox::popo::createNotificationCallback(RequestHandler)).or_else([](auto) {
        std::cerr << "unable to attach myServerLeft event" << std::endl;
        std::exit(EXIT_FAILURE);
    });

    shutdownSemaphore.wait().or_else(
        [](auto) { std::cerr << "unable to call wait on shutdownSemaphore - semaphore corrupt?" << std::endl; });
    
    listener.detachEvent(myServerLeft, iox::popo::ServerEvent::DATA_RECEIVED);
    return (EXIT_SUCCESS);
}