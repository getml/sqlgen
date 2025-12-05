#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <string_view>
#include <sqlgen/postgres.hpp>

namespace {

using namespace sqlgen;
using namespace sqlgen::postgres;

// Simple helper to wait for notifications with timeout
std::vector<Notification> wait_for_notifications(
    Connection& conn,
    std::chrono::milliseconds timeout = std::chrono::milliseconds{1000}
) {
    auto wait_res = conn.wait_for_notification(timeout);
    if (!wait_res || *wait_res != NotificationWaitResult::Ready) {
        return {};
    }
    return conn.get_notifications();
}

} // anonymous namespace

TEST(postgres, basic_listen_notify) {
  using namespace sqlgen;
  using namespace sqlgen::postgres;

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  auto listener = postgres::connect(credentials);
  auto sender = postgres::connect(credentials);

  ASSERT_TRUE(listener);
  ASSERT_TRUE(sender);

  // Listener subscribes to channel
  auto listen_res = listener.listen("test_channel");
  ASSERT_TRUE(listen_res) << listen_res.error().what();

  // Sender sends a notification
  auto notify_res = sender.notify("test_channel", "hello world");
  ASSERT_TRUE(notify_res) << notify_res.error().what();

  // Listener waits and receives
  auto notifications = wait_for_notifications(listener);
  ASSERT_EQ(notifications.size(), 1);
  EXPECT_EQ(notifications[0].channel, "test_channel");
  EXPECT_EQ(notifications[0].payload, "hello world");
  EXPECT_GT(notifications[0].backend_pid, 0);
}

TEST(postgres, notify_without_listener_is_silent) {
  using namespace sqlgen;
  using namespace sqlgen::postgres;

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  auto sender = postgres::connect(credentials);
  ASSERT_TRUE(sender);

  // Notify on a channel with no listener → should not error
  auto res = sender.notify("unused_channel", "payload");
  ASSERT_TRUE(res) << res.error().what();
}

TEST(postgres, InvalidChannelNameRejected) {
  using namespace sqlgen;
  using namespace sqlgen::postgres;

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  auto conn = postgres::connect(creds);
  ASSERT_TRUE(conn);

  // Invalid: starts with digit
  EXPECT_FALSE(conn.listen("123chan"));
  EXPECT_FALSE(conn.notify("123chan"));

  // Invalid: contains hyphen
  EXPECT_FALSE(conn.listen("my-chan"));
  EXPECT_FALSE(conn.notify("my-chan"));

  // Valid: underscore + alphanumeric
  EXPECT_TRUE(conn.listen("_chan1"));
  EXPECT_TRUE(conn.unlisten("_chan1"));
}

TEST(postgres, unlisten_star) {
  using namespace sqlgen;
  using namespace sqlgen::postgres;

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};

  auto conn = postgres::connect(credentials);
  ASSERT_TRUE(conn);

  ASSERT_TRUE(conn.listen("chan_a"));
  ASSERT_TRUE(conn.listen("chan_b"));

  // Unlisten all
  ASSERT_TRUE(conn.unlisten("*"));

  // Notify won't be received, but we just verify no error
  auto sender = postgres::connect(creds);
  ASSERT_TRUE(sender.notify("chan_a", "test"));
}

TEST(postgres, multiple_notifications_in_burst) {
  using namespace sqlgen;
  using namespace sqlgen::postgres;

  const auto credentials = sqlgen::postgres::Credentials{.user = "postgres",
                                                         .password = "password",
                                                         .host = "localhost",
                                                         .dbname = "postgres"};
    auto listener = postgres::connect(credentials);
    auto sender = postgres::connect(credentials);

    ASSERT_TRUE(listener);
    ASSERT_TRUE(sender);

    const std::string channel = "burst_channel";
    ASSERT_TRUE(listener.listen(channel));

    const int num_notifications = 5;
    std::vector<std::string> expected_payloads;
    for (int i = 0; i < num_notifications; ++i) {
        const std::string payload = "msg_" + std::to_string(i);
        expected_payloads.push_back(payload);
        ASSERT_TRUE(sender.notify(channel, payload));
        // Small delay to improve reliability on slow CI (optional but safe)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Wait once — PostgreSQL typically delivers all in one socket read
    auto notifications = wait_for_notifications(listener, std::chrono::milliseconds{2000});

    ASSERT_EQ(notifications.size(), num_notifications)
        << "Expected " << num_notifications << " notifications, got " << notifications.size();

    // Payloads may arrive in order (PG guarantees per-backend order)
    for (int i = 0; i < num_notifications; ++i) {
        EXPECT_EQ(notifications[i].channel, channel);
        EXPECT_EQ(notifications[i].payload, expected_payloads[i]);
        EXPECT_GT(notifications[i].backend_pid, 0);
    }
}
