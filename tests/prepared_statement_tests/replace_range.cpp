#include <sqlite_orm/sqlite_orm.h>
#include <catch2/catch.hpp>

#include "prepared_common.h"

using namespace sqlite_orm;

TEST_CASE("Prepared replace range") {
    using namespace PreparedStatementTests;
    using Catch::Matchers::UnorderedEquals;

    const int defaultVisitTime = 50;

    auto filename = "prepared.sqlite";
    remove(filename);
    auto storage = make_storage(filename,
                                make_index("user_id_index", &User::id),
                                make_table("users",
                                           make_column("id", &User::id, primary_key(), autoincrement()),
                                           make_column("name", &User::name)),
                                make_table("visits",
                                           make_column("id", &Visit::id, primary_key(), autoincrement()),
                                           make_column("user_id", &Visit::userId),
                                           make_column("time", &Visit::time, default_value(defaultVisitTime)),
                                           foreign_key(&Visit::userId).references(&User::id)),
                                make_table("users_and_visits",
                                           make_column("user_id", &UserAndVisit::userId),
                                           make_column("visit_id", &UserAndVisit::visitId),
                                           make_column("description", &UserAndVisit::description),
                                           primary_key(&UserAndVisit::userId, &UserAndVisit::visitId)));
    storage.sync_schema();

    storage.replace(User{1, "Team BS"});
    storage.replace(User{2, "Shy'm"});
    storage.replace(User{3, "Maître Gims"});

    storage.replace(UserAndVisit{2, 1, "Glad you came"});
    storage.replace(UserAndVisit{3, 1, "Shine on"});

    std::vector<User> users;
    std::vector<User> expected;
    SECTION("empty") {
        expected.push_back(User{1, "Team BS"});
        expected.push_back(User{2, "Shy'm"});
        expected.push_back(User{3, "Maître Gims"});
        try {
            auto statement = storage.prepare(replace_range(users.begin(), users.end()));
            REQUIRE(false);
        } catch(const std::system_error& e) {
            //..
        }
    }
    SECTION("one existing") {
        User user{1, "Raye"};
        expected.push_back(user);
        expected.push_back(User{2, "Shy'm"});
        expected.push_back(User{3, "Maître Gims"});
        users.push_back(user);
        auto statement = storage.prepare(replace_range(users.begin(), users.end()));
        REQUIRE(get<0>(statement) == users.begin());
        REQUIRE(get<1>(statement) == users.end());
        storage.execute(statement);
    }
    SECTION("one existing and one new") {
        User user{2, "Raye"};
        User user2{4, "Bebe Rexha"};
        expected.push_back(User{1, "Team BS"});
        expected.push_back(user);
        expected.push_back(User{3, "Maître Gims"});
        expected.push_back(user2);
        users.push_back(user);
        users.push_back(user2);
        auto statement = storage.prepare(replace_range(users.begin(), users.end()));
        REQUIRE(get<0>(statement) == users.begin());
        REQUIRE(get<1>(statement) == users.end());
        storage.execute(statement);
    }
    SECTION("All existing") {
        users.push_back(User{1, "Selena Gomez"});
        users.push_back(User{2, "Polina"});
        users.push_back(User{3, "Polina"});
        expected = users;
        auto statement = storage.prepare(replace_range(users.begin(), users.end()));
        REQUIRE(get<0>(statement) == users.begin());
        REQUIRE(get<1>(statement) == users.end());
        storage.execute(statement);
    }
    auto rows = storage.get_all<User>();
    REQUIRE_THAT(rows, UnorderedEquals(expected));
}
