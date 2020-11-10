// Copyright 2018, Bosch Software Innovations GmbH.
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

#include "rclcpp_generic/generic_subscription.hpp"

#include <memory>
#include <string>
#include <utility>

#include "rcl/subscription.h"
#include "rclcpp_generic/typesupport_helpers.hpp"

namespace
{
rcl_subscription_options_t rosbag2_get_subscription_options(const rclcpp::QoS & qos)
{
  auto options = rcl_subscription_get_default_options();
  options.qos = qos.get_rmw_qos_profile();
  return options;
}
}  // unnamed namespace

namespace rclcpp_generic
{

std::shared_ptr<GenericSubscription> GenericSubscription::create(
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface,
  const std::string & topic_name,
  const std::string & topic_type,
  const rclcpp::QoS & qos,
  std::function<void(std::shared_ptr<rclcpp::SerializedMessage>)> callback,
  rclcpp::CallbackGroup::SharedPtr group)
{
  auto ts_lib = rclcpp_generic::get_typesupport_library(
    topic_type, "rosidl_typesupport_cpp");

  std::shared_ptr<GenericSubscription> subscription(new GenericSubscription(
      topics_interface->get_node_base_interface(),
      std::move(ts_lib),
      topic_name,
      topic_type,
      qos,
      callback));

  topics_interface->add_subscription(subscription, std::move(group));

  return subscription;
}

GenericSubscription::GenericSubscription(
  rclcpp::node_interfaces::NodeBaseInterface * node_base,
  const std::shared_ptr<rcpputils::SharedLibrary> ts_lib,
  const std::string & topic_name,
  const std::string & topic_type,
  const rclcpp::QoS & qos,
  std::function<void(std::shared_ptr<rclcpp::SerializedMessage>)> callback)
: SubscriptionBase(
    node_base,
    *rclcpp_generic::get_typesupport_handle(
      topic_type, "rosidl_typesupport_cpp", ts_lib),
    topic_name,
    rosbag2_get_subscription_options(qos),
    true),
  callback_(callback),
  qos_(qos)
{}

std::shared_ptr<void> GenericSubscription::create_message()
{
  return create_serialized_message();
}

std::shared_ptr<rclcpp::SerializedMessage> GenericSubscription::create_serialized_message()
{
  return std::make_shared<rclcpp::SerializedMessage>(0);
}

void GenericSubscription::handle_message(
  std::shared_ptr<void> & message, const rclcpp::MessageInfo & message_info)
{
  (void) message_info;
  auto typed_message = std::static_pointer_cast<rclcpp::SerializedMessage>(message);
  callback_(typed_message);
}

void GenericSubscription::handle_loaned_message(
  void * message, const rclcpp::MessageInfo & message_info)
{
  (void) message;
  (void) message_info;
}

void GenericSubscription::return_message(std::shared_ptr<void> & message)
{
  auto typed_message = std::static_pointer_cast<rclcpp::SerializedMessage>(message);
  return_serialized_message(typed_message);
}

void GenericSubscription::return_serialized_message(
  std::shared_ptr<rclcpp::SerializedMessage> & message)
{
  message.reset();
}

const rclcpp::QoS & GenericSubscription::qos_profile() const
{
  return qos_;
}

}  // namespace rclcpp_generic
