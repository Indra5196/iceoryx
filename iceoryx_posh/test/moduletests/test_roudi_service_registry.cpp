// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI. All rights reserved.
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

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::roudi;
/// @todo #415 Replace Wildcards once service registry has its new data structure
class ServiceRegistry_test : public Test
{
  public:
    using ServiceDescription = iox::capro::ServiceDescription;
    virtual void SetUp()
    {
        internal::CaptureStdout();
    }

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
    iox::roudi::ServiceRegistry sut;

    iox::roudi::ServiceRegistry::ServiceDescriptionVector_t searchResults;
};

TEST_F(ServiceRegistry_test, AddNoServiceDescriptionsAndWildcardSearchReturnsNothing)
{
    sut.find(searchResults, Wildcard, Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddMaximumNumberOfServiceDescriptionsWorks)
{
    iox::cxx::vector<ServiceDescription, ServiceRegistry::MAX_SERVICE_DESCRIPTIONS> services;

    for (uint64_t i = 0U; i < ServiceRegistry::MAX_SERVICE_DESCRIPTIONS; i++)
    {
        services.push_back(iox::capro::ServiceDescription(
            "Foo", "Bar", iox::capro::IdString_t(iox::cxx::TruncateToCapacity, iox::cxx::convert::toString(i))));
    }

    for (auto& service : services)
    {
        auto result = sut.add(service);
        ASSERT_FALSE(result.has_error());
    }
}

TEST_F(ServiceRegistry_test, AddMoreThanMaximumNumberOfServiceDescriptionsFails)
{
    iox::cxx::vector<ServiceDescription, ServiceRegistry::MAX_SERVICE_DESCRIPTIONS> services;

    for (uint64_t i = 0U; i < ServiceRegistry::MAX_SERVICE_DESCRIPTIONS; i++)
    {
        services.push_back(iox::capro::ServiceDescription(
            "Foo", "Bar", iox::capro::IdString_t(iox::cxx::TruncateToCapacity, iox::cxx::convert::toString(i))));
    }

    for (auto& service : services)
    {
        auto result = sut.add(service);
        ASSERT_FALSE(result.has_error());
    }

    auto result = sut.add(iox::capro::ServiceDescription("Foo", "Bar", "Baz"));
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ServiceRegistry::Error::SERVICE_REGISTRY_FULL));
}

TEST_F(ServiceRegistry_test, AddServiceDescriptionsWhichWasAlreadyAddedAndReturnsOneResult)
{
    auto result1 = sut.add(ServiceDescription("Li", "La", "Launebaer"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = sut.add(ServiceDescription("Li", "La", "Launebaer"));
    ASSERT_FALSE(result2.has_error());

    sut.find(searchResults, Wildcard, Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Li", "La", "Launebaer")));
    EXPECT_THAT(searchResults[0].referenceCounter, Eq(2));
}

TEST_F(ServiceRegistry_test, AddServiceDescriptionsTwiceAndRemoveOnceAndReturnsOneResult)
{
    auto result1 = sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result1.has_error());

    auto result2 = sut.add(ServiceDescription("Li", "La", "Launebaerli"));
    ASSERT_FALSE(result2.has_error());

    sut.remove(ServiceDescription("Li", "La", "Launebaerli"));

    sut.find(searchResults, Wildcard, Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Li", "La", "Launebaerli")));
    EXPECT_THAT(searchResults[0].referenceCounter, Eq(1));
}

TEST_F(ServiceRegistry_test, AddInvalidServiceDescriptionsWorks)
{
    auto result = sut.add(ServiceDescription());
    ASSERT_FALSE(result.has_error());
}

TEST_F(ServiceRegistry_test, RemovingServiceDescriptionsWhichWasntAddedFails)
{
    sut.remove(ServiceDescription("Sim", "Sa", "Lambim"));
    EXPECT_THAT(sut.getServices().size(), Eq(0));
}

TEST_F(ServiceRegistry_test, RemovingInvalidServiceDescriptionsWorks)
{
    ASSERT_FALSE(sut.add(ServiceDescription()).has_error());
    sut.remove(ServiceDescription());
    EXPECT_THAT(sut.getServices().size(), Eq(0));
}

TEST_F(ServiceRegistry_test, SingleInvalidServiceDescriptionsCanBeFoundWithWildcardSearch)
{
    ASSERT_FALSE(sut.add(ServiceDescription()).has_error());
    sut.find(searchResults, Wildcard, Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription()));
}

TEST_F(ServiceRegistry_test, SingleInvalidServiceDescriptionsCanBeFoundWithEmptyString)
{
    ASSERT_FALSE(sut.add(ServiceDescription()).has_error());
    sut.find(searchResults, "", "");

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription()));
}

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithWildcardSearch)
{
    auto result = sut.add(ServiceDescription("Foo", "Bar", "Baz"));
    ASSERT_FALSE(result.has_error());
    sut.find(searchResults, Wildcard, Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Foo", "Bar", "Baz")));
}

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithInstanceName)
{
    auto result = sut.add(ServiceDescription("Baz", "Bar", "Foo"));
    ASSERT_FALSE(result.has_error());
    sut.find(searchResults, Wildcard, "Bar");

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(ServiceDescription("Baz", "Bar", "Foo")));
}

TEST_F(ServiceRegistry_test, SingleServiceDescriptionCanBeFoundWithServiceName)
{
    iox::capro::ServiceDescription service1("a", "b", "c");
    ASSERT_FALSE(sut.add(service1).has_error());
    sut.find(searchResults, "a", Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service1));
}

TEST_F(ServiceRegistry_test, ValidAndInvalidServiceDescriptionsCanAllBeFoundWithWildcardSearch)
{
    ServiceDescription service1;
    ServiceDescription service2("alpha", "bravo", "charlie");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    sut.find(searchResults, Wildcard, Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(2));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service1));
    EXPECT_THAT(searchResults[1].serviceDescription, Eq(service2));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameCanAllBeFound)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    sut.find(searchResults, "a", Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(3));

    bool hasFoundB = false;
    bool hasFoundC = false;
    bool hasFoundD = false;

    for (auto& e : searchResults)
    {
        if (e.serviceDescription == service1)
            hasFoundB = true;
        if (e.serviceDescription == service2)
            hasFoundC = true;
        if (e.serviceDescription == service3)
            hasFoundD = true;
    }

    EXPECT_THAT(hasFoundB && hasFoundC && hasFoundD, Eq(true));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithDifferentServiceNameCanAllBeFound)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("c", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    sut.find(searchResults, "a", Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service1));
    searchResults.clear();

    sut.find(searchResults, "c", Wildcard);
    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service2));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionWithSameServiceNameFindsSpecificService)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    sut.find(searchResults, "a", "c");

    EXPECT_THAT(searchResults.size(), Eq(1));
    EXPECT_THAT(searchResults[0].serviceDescription, Eq(service2));
}

TEST_F(ServiceRegistry_test, MultipleServiceDescriptionAddedInNonLinearOrderFindsCorrectServices)
{
    iox::capro::ServiceDescription service1("a", "1", "moep");
    iox::capro::ServiceDescription service2("b", "2", "moep");
    iox::capro::ServiceDescription service3("c", "3", "moep");
    iox::capro::ServiceDescription service4("d", "4", "moep");
    iox::capro::ServiceDescription service5("e", "5", "moep");

    ASSERT_FALSE(sut.add(service5).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    ASSERT_FALSE(sut.add(service4).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service1).has_error());

    sut.remove(service5);
    sut.remove(service1);
    EXPECT_THAT(sut.getServices().size(), Eq(3));
    sut.find(searchResults, "a", Wildcard);

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, FindSpecificNonExistingServiceDescriptionFails)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    sut.find(searchResults, "a", "g");

    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingMultipleServiceDescriptionWithSameServicesAndRemovingSpecificDoesNotFindSpecific)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    sut.remove(service2);
    EXPECT_THAT(sut.getServices().size(), Eq(2));

    sut.find(searchResults, "a", "c");
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, ServiceNotFoundAfterAddingAndRemovingToServiceRegistry)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("b", "c", "c");
    iox::capro::ServiceDescription service3("c", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    sut.remove(service2);
    EXPECT_THAT(sut.getServices().size(), Eq(2));

    sut.find(searchResults, "b", "c");
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingMultipleServiceDescriptionAndRemovingAllDoesNotFindAnything)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");

    ASSERT_FALSE(sut.add(service1).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());

    sut.remove(service1);
    sut.remove(service2);
    sut.remove(service3);

    sut.find(searchResults, "a", Wildcard);
    EXPECT_THAT(searchResults.size(), Eq(0));
}

TEST_F(ServiceRegistry_test, AddingVariousServiceDescriptionAndGetServicesDoesNotReturnDuplicate)
{
    iox::capro::ServiceDescription service1("a", "b", "b");
    iox::capro::ServiceDescription service2("a", "c", "c");
    iox::capro::ServiceDescription service3("a", "d", "d");
    iox::capro::ServiceDescription service4("e", "f", "f");

    ASSERT_FALSE(sut.add(service1).has_error());
    // add same service a, instance c to check if in sut only one entry is created
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service2).has_error());
    ASSERT_FALSE(sut.add(service3).has_error());
    ASSERT_FALSE(sut.add(service4).has_error());

    auto serviceDescriptionVector = sut.getServices();

    bool service1Found = false;
    bool service2Found = false;
    bool service4Found = false;

    for (auto const& element : serviceDescriptionVector)
    {
        if (element.serviceDescription == service1)
        {
            service1Found = true;
        }

        if (element.serviceDescription == service2)
        {
            service2Found = true;
        }

        if (element.serviceDescription == service4)
        {
            service4Found = true;
        }
    }
    EXPECT_THAT(serviceDescriptionVector.size(), Eq(4));
    EXPECT_THAT(service1Found && service2Found && service4Found, Eq(true));
}

} // namespace
