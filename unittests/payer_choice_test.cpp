#include <algorithm>
#include <test_contracts.hpp>

#include <sysio/chain/config.hpp>
#include <sysio/chain/resource_limits.hpp>
#include <sysio/chain/config.hpp>
#include <sysio/testing/chainbase_fixture.hpp>

#include <boost/test/unit_test.hpp>
#include <sysio/testing/tester.hpp>

using namespace sysio::testing;
using namespace sysio::chain;

BOOST_AUTO_TEST_SUITE(payer_choice_test)

    /**
     * Test to ensure that a user without ram calling a non-whitelisted contract fails
     */
    BOOST_AUTO_TEST_CASE(no_resources_no_love) try {
        tester c(setup_policy::full);
        c.produce_block();

        const auto &tester1_account = account_name("tester1");
        const auto &alice_account = account_name("alice");
        const auto &bob_account = account_name("bob");

        c.create_accounts({tester1_account, alice_account, bob_account});
        c.produce_block();
        c.set_code(tester1_account, test_contracts::ram_restrictions_test_wasm());
        c.set_abi(tester1_account, test_contracts::ram_restrictions_test_abi());

        c.produce_block();

        fc::logger::get("default").set_log_level(fc::log_level::debug);
        wlog("No Resource Testing");

        wlog("Attempt with no resources should fail");
        BOOST_REQUIRE_EXCEPTION(
            c.push_action(tester1_account, "setdata"_n, alice_account, mutable_variant_object()
                ("len1", 10)
                ("len2", 0)
                ("payer", alice_account)
            ),
            resource_exhausted_exception,
            fc_exception_message_contains("can pay")
        );

        c.register_node_owner(alice_account, 2);
        c.produce_block();

        wlog("Attempt with node owner as caller should fail without sysio.payer permission");
        // TODO: this should fail because contract does not have resources
        // Even though the caller has the resources, they have not passed the sysio.payer permission.
        BOOST_REQUIRE_EXCEPTION(
            c.push_action(tester1_account, "setdata"_n, {
                permission_level(alice_account, "active"_n)
                }, mutable_variant_object()
                ("len1", 10)
                ("len2", 0)
                ("payer", alice_account)
            ),
            resource_exhausted_exception,
            fc_exception_message_contains("can pay")
        );
        c.produce_block();

        wlog("Attempt by node owner with sysio.payer permission should succeed");
        // TODO: this should pass, relying on the resource capacity of the caller, per the sysio.payer permission
        // push_action again, but this time adding the sysio_payer permission
        c.push_action(tester1_account, "setdata"_n, {
                          permission_level(alice_account, "active"_n),
                          permission_level(alice_account, config::sysio_payer_name)
                      }, mutable_variant_object()
                      ("len1", 10)
                      ("len2", 0)
                      ("payer", alice_account)
        );

        c.produce_block();
    } FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_SUITE_END()
