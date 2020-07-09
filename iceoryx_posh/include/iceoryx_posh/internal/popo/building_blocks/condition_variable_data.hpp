// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_DATA_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_DATA_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

namespace iox
{
namespace popo
{
struct ConditionVariableData
{
    ConditionVariableData() noexcept = default;
    ConditionVariableData(const ConditionVariableData& rhs) = delete;
    ConditionVariableData(ConditionVariableData&& rhs) = default;
    ConditionVariableData& operator=(const ConditionVariableData& rhs) = delete;
    ConditionVariableData& operator=(ConditionVariableData&& rhs) = default;

    posix::Semaphore m_semaphore =
        std::move(posix::Semaphore::create(0u)
                      .or_else([](posix::SemaphoreError&) {
                          errorHandler(Error::kPOPO__CONDITION_VARIABLE_DATA_FAILED_TO_CREATE_SEMAPHORE,
                                       nullptr,
                                       ErrorLevel::FATAL);
                      })
                      .get_value());
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_DATA_HPP
