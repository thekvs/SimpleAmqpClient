#include "connected_test.h"

using namespace AmqpClient;

TEST_F(connected_test, first_channel)
{
  channel->DeclareExchange("test_channel_exchange", Channel::EXCHANGE_TYPE_FANOUT, false, false, true);
  channel->DeleteExchange("test_channel_exchange");
}

// Check to see that channels are reused properly
TEST_F(connected_test, channel_reuse)
{
  channel->DeclareExchange("test_channel_exchange1", Channel::EXCHANGE_TYPE_FANOUT, false, false, true);
  channel->DeclareExchange("test_channel_exchange2", Channel::EXCHANGE_TYPE_FANOUT, false, false, true);
  channel->DeleteExchange("test_channel_exchange1");
  channel->DeleteExchange("test_channel_exchange2");
}

// Check to see that a new channel is created when a channel is put in an exception state
TEST_F(connected_test, channel_recover_from_error)
{
  EXPECT_THROW(channel->DeclareExchange("test_channel_exchangedoesnotexist", Channel::EXCHANGE_TYPE_FANOUT, true, false, true), ChannelException);

  channel->DeclareExchange("test_channel_exchange", Channel::EXCHANGE_TYPE_FANOUT, false, false, true);
  channel->DeleteExchange("test_channel_exchange");
}

TEST_F(connected_test, channel_publish_success1)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");

  channel->BasicPublish("", "test_channel_routingkey", message, false, false);
}

TEST_F(connected_test, channel_publish_success2)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");

  channel->BasicPublish("", "test_channel_routingkey", message, false, false);
  channel->BasicPublish("", "test_channel_routingkey", message, false, false);
}

TEST_F(connected_test, channel_publish_returned_mandatory)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");

  EXPECT_THROW(channel->BasicPublish("", "test_channel_noqueue", message, true, false), MessageReturnedException);
}

TEST_F(connected_test, channel_publish_returned_immediate)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");
  std::string queue_name = channel->DeclareQueue("");

  EXPECT_THROW(channel->BasicPublish("", queue_name, message, false, true), MessageReturnedException);
}

TEST_F(connected_test, channel_publish_bad_exchange)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");

  EXPECT_THROW(channel->BasicPublish("test_channel_badexchange", "test_channel_rk", message, false, false), ChannelException);
}

TEST_F(connected_test, channel_publish_bad_exchange_recover)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");

  EXPECT_THROW(channel->BasicPublish("test_channel_badexchange", "test_channel_rk", message, false, false), ChannelException);

  channel->BasicPublish("", "test_channel_rk", message, false, false);
}

TEST_F(connected_test, channel_consume_success)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");
  std::string queue = channel->DeclareQueue("");
  channel->BasicPublish("", queue, message);

  std::string consumer = channel->BasicConsume(queue);

  Envelope::ptr_t consumed_envelope;
  EXPECT_TRUE(channel->BasicConsumeMessage(consumer, consumed_envelope));
}

TEST_F(connected_test, channel_consume_success_timeout)
{
  BasicMessage::ptr_t message = BasicMessage::Create("Test message");
  std::string queue = channel->DeclareQueue("");

  std::string consumer = channel->BasicConsume(queue, "", true, false);
  channel->BasicPublish("", queue, message);

  Envelope::ptr_t consumed_envelope;
  EXPECT_TRUE(channel->BasicConsumeMessage(consumer, consumed_envelope, 1));
}

TEST(test_channels, big_message)
{
  Channel::ptr_t channel = Channel::Create(connected_test::GetBrokerHost(), 5672, "guest", "guest", "/", 4096);
  BasicMessage::ptr_t message = BasicMessage::Create(std::string(4099, 'a'));

  std::string queue = channel->DeclareQueue("");

  std::string consumer = channel->BasicConsume(queue);
  channel->BasicPublish("", queue, message);

  Envelope::ptr_t consumed_envelope;
  EXPECT_TRUE(channel->BasicConsumeMessage(consumer, consumed_envelope));
}
