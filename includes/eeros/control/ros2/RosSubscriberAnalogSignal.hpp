#pragma once

#include <eeros/control/ros2/RosSubscriber.hpp>
#include <eeros_msgs/msg/analog_signal.hpp>

namespace eeros {
namespace control {

/**
 * This block allows to subscribe to a ROS message of type eeros_msgs::msg::AnalogSignal
 * and publishes it as a signal of type double or vector of double.
 *
 * @tparam SigOutType - type of the output signal
 *
 * @since v1.0
 */
template < typename SigOutType = double >
class RosSubscriberAnalogSignal : public RosSubscriber<eeros_msgs::msg::AnalogSignal, 1, SigOutType> {
  typedef eeros_msgs::msg::AnalogSignal TRosMsg;

 public:
  /**
   * Creates an instance of a ROS subscriber block. The block reads
   * ROS messages of type eeros_msgs::msg::AnalogSignal under a given topic
   * and outputs its values onto a signal of type double or vector of double.
   * If several messages are pending for a given topic
   * all the messages are consumed and the signal is set to the
   * newest.
   *
   * @param node - ROS node as a shared ptr
   * @param topic - name of the topic
   * @param syncWithTopic - when set to true the executor runs all time domains upon receiving this message
   * @param queueSize - maximum number of incoming messages to be queued for delivery to subscribers
   */
  RosSubscriberAnalogSignal(const rclcpp::Node::SharedPtr node, const std::string& topic, bool syncWithTopic = false,
                      const uint32_t queueSize = 1000)
      : RosSubscriber<TRosMsg, 1, SigOutType>(node, topic, syncWithTopic, queueSize) {}

  /**
   * Disabling use of copy constructor because the block should never be copied unintentionally.
   */
  RosSubscriberAnalogSignal(const RosSubscriberAnalogSignal& other) = delete;

  /**
   * This function is called whenever the run function reads the
   * next pending ROS message.
   *
   * @param msg - message content
   */
  virtual void parseMsg(const TRosMsg& msg) {
    _set<SigOutType>(msg);
    this->out.getSignal().setTimestamp(RosTools::convertToEerosTime(msg.timestamp));
  }

 private:
  template <typename S> typename std::enable_if<std::is_floating_point<S>::value>::type _set(const TRosMsg& msg) {
    this->out.getSignal().setValue(msg.val[0]);
  }
  template <typename S> typename std::enable_if<std::is_compound<S>::value>::type _set(const TRosMsg& msg) {
    SigOutType val;
    for (int i = 0; i < val.size(); i++)
      val[i] = msg.val[i];
    this->out.getSignal().setValue(val);
  }

};

/********** Print functions **********/
std::ostream& operator<<(std::ostream& os, RosSubscriberAnalogSignal<>& s) {
  os << "Block RosSubscriberAnalogSignal: '" << s.getName();
  return os;
}

}
}