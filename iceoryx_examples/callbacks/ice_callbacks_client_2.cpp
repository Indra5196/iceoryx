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

#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_handler.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

iox::posix::Semaphore shutdownSemaphore =
    iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();

bool keepRunning = true;
constexpr char APP_NAME[] = "iox-cpp-callbacks-client-ii";

static void sigHandler(int f_sig IOX_MAYBE_UNUSED)
{
    shutdownSemaphore.post().or_else([](auto) {
        std::cerr << "unable to call post on shutdownSemaphore - semaphore corrupt?" << std::endl;
        std::exit(EXIT_FAILURE);
    });
    keepRunning = false;
}

void AdjustMethod(uint32_t responseMsg){
    std::cout << "Response: " << responseMsg << "\n";
}

void RunResponseHandler(iox::popo::Client<uint32_t, uint32_t>* client){
    client->takeResponses().and_then([client](auto& sample) {
        client->runResponseHandler(sample);
    });
}

void sending(iox::popo::Client<uint32_t, uint32_t>& myClientLeft)
{
    for (uint32_t counter = 0U; keepRunning; ++counter)
    {
        myClientLeft.loanRequest(sizeof(uint32_t)).and_then([&](auto& sample) {
                sample.getUserHeader().setMethodId(0x1);
                *sample = counter;
                std::cout << "Client 2 Sending: " << counter << "\n";
                myClientLeft.sendRequest(std::move(sample));
            })
            .or_else([&](auto& error) {
                // Do something with the error
                std::cerr << "Unable to loan sample, error code: " << static_cast<uint64_t>(error) << std::endl;
            });
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    std::cout << "Runtime Initialized client\n";
    iox::popo::Listener listener;
    std::cout << "Listener Created client\n";
    iox::popo::Client<uint32_t, uint32_t> myClientLeft({"Radar", "FrontLeft"});
    std::cout << "Client Created\n";
    myClientLeft.registerResponseHandler(AdjustMethod);

    listener.attachEvent(myClientLeft, iox::popo::ClientEvent::DATA_RECEIVED, iox::popo::createNotificationCallback(RunResponseHandler)).or_else([](auto) {
        std::cerr << "unable to attach myClientLeft event" << std::endl;
        std::exit(EXIT_FAILURE);
    });

    std::thread tx(sending, std::ref(myClientLeft));

    shutdownSemaphore.wait().or_else(
        [](auto) { std::cerr << "unable to call wait on shutdownSemaphore - semaphore corrupt?" << std::endl; });
    
    tx.join();

    listener.detachEvent(myClientLeft, iox::popo::ClientEvent::DATA_RECEIVED);
    return (EXIT_SUCCESS);
}
