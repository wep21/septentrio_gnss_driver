// *****************************************************************************
//
// © Copyright 2020, Septentrio NV/SA.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    3. Neither the name of the copyright holder nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// *****************************************************************************

// *****************************************************************************
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:

// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// *****************************************************************************

#ifndef CALLBACK_HANDLERS_HPP
#define CALLBACK_HANDLERS_HPP

// Boost includes
#include <boost/foreach.hpp>
// In C++, writing a loop that iterates over a sequence is tedious -->
// BOOST_FOREACH(char ch, "Hello World")
#include <boost/function.hpp>
// E.g. boost::function<int(const char*)> f = std::atoi;defines a pointer f that can
// point to functions that expect a parameter of type const char* and return a value
// of type int Generally, any place in which a function pointer would be used to
// defer a call or make a callback, Boost.Function can be used instead to allow the
// user greater flexibility in the implementation of the target.
#include <boost/thread.hpp>
// Boost's thread enables the use of multiple threads of execution with shared data
// in portable C++ code. It provides classes and functions for managing the threads
// themselves, along with others for synchronizing data between the threads or
// providing separate copies of data specific to individual threads.
#include <boost/thread/condition.hpp>
#include <boost/tokenizer.hpp>
// The tokenizer class provides a container view of a series of tokens contained in a
// sequence, e.g. if you are not interested in non-words...
#include <boost/algorithm/string/join.hpp>
// Join algorithm is a counterpart to split algorithms. It joins strings from a
// 'list' by adding user defined separator.
#include <boost/date_time/posix_time/posix_time.hpp>
// The class boost::posix_time::ptime that we will use defines a location-independent
// time. It uses the type boost::gregorian::date, yet also stores a time.
#include <boost/asio.hpp>
// Boost.Asio may be used to perform both synchronous and asynchronous operations on
// I/O objects such as sockets.
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>

// ROSaic and C++ includes
#include <algorithm>
#include <septentrio_gnss_driver/communication/rx_message.hpp>

/**
 * @file callback_handlers.hpp
 * @date 22/08/20
 * @brief Handles callbacks when reading NMEA/SBF messages
 */

extern bool g_channelstatus_has_arrived_gpsfix;
extern bool g_measepoch_has_arrived_gpsfix;
extern bool g_dop_has_arrived_gpsfix;
extern bool g_pvtgeodetic_has_arrived_gpsfix;
extern bool g_pvtgeodetic_has_arrived_navsatfix;
extern bool g_pvtgeodetic_has_arrived_pose;
extern bool g_poscovgeodetic_has_arrived_gpsfix;
extern bool g_poscovgeodetic_has_arrived_navsatfix;
extern bool g_poscovgeodetic_has_arrived_pose;
extern bool g_velcovgeodetic_has_arrived_gpsfix;
extern bool g_atteuler_has_arrived_gpsfix;
extern bool g_atteuler_has_arrived_pose;
extern bool g_attcoveuler_has_arrived_gpsfix;
extern bool g_attcoveuler_has_arrived_pose;
extern bool g_insnavgeod_has_arrived_gpsfix;
extern bool g_insnavgeod_has_arrived_navsatfix;
extern bool g_insnavgeod_has_arrived_pose;
extern bool g_receiverstatus_has_arrived_diagnostics;
extern bool g_qualityind_has_arrived_diagnostics;

extern bool g_publish_navsatfix;
extern bool g_publish_gpsfix;
extern bool g_publish_gpst;
extern bool g_publish_pose;
extern bool g_publish_diagnostics;
extern bool g_response_received;
extern boost::mutex g_response_mutex;
extern boost::condition_variable g_response_condition;
extern bool g_cd_received;
extern boost::mutex g_cd_mutex;
extern boost::condition_variable g_cd_condition;
extern uint32_t g_cd_count;
extern std::string g_rx_tcp_port;

namespace io_comm_rx {
    /**
     * @class CallbackHandler
     * @brief Abstract class representing a generic callback handler, includes
     * high-level functionality such as wait
     */
    class AbstractCallbackHandler
    {
    public:
        virtual void handle(RxMessage& rx_message, std::string message_key) = 0;

        bool Wait(const boost::posix_time::time_duration& timeout)
        {
            boost::mutex::scoped_lock lock(mutex_);
            return condition_.timed_wait(lock, timeout);
        }

    protected:
        boost::mutex mutex_;
        boost::condition_variable condition_;
    };

    /**
     * @class CallbackHandler
     * @brief Derived class operating on a ROS message level
     */
    template <typename T>
    class CallbackHandler : public AbstractCallbackHandler
    {
    public:
        virtual const T& Get() { return message_; }

        void handle(RxMessage& rx_message, std::string message_key)
        {
            boost::mutex::scoped_lock lock(mutex_);
            try
            {
                if (!rx_message.read(message_key))
                {
                    std::ostringstream ss;
                    ss << "Rx decoder error for message with ID (empty field if non-determinable)"
                       << rx_message.messageID() << ". Reason unknown.";
                    throw std::runtime_error(ss.str());
                    return;
                }
            } catch (std::runtime_error& e)
            {
                std::ostringstream ss;
                ss << "Rx decoder error for message with ID "
                   << rx_message.messageID() << ".\n"
                   << e.what();
                throw std::runtime_error(ss.str());
                return;
            }

            condition_.notify_all();
        }

    private:
        T message_;
    };

    /**
     * @class CallbackHandlers
     * @brief Represents ensemble of (to be constructed) ROS messages, to be handled
     * at once by this class
     */
    class CallbackHandlers
    {

    public:
        //! Key is std::string and represents the ROS message key, value is
        //! boost::shared_ptr<CallbackHandler>
        typedef std::multimap<std::string,
                              boost::shared_ptr<AbstractCallbackHandler>>
            CallbackMap;

        CallbackHandlers() = default;

        /**
         * @brief Adds a pair to the multimap "callbackmap_", with the message_key
         * being the key
         *
         * This method is called by "handlers_" in rosaic_node.cpp.
         * T would be a (custom or not) ROS message, e.g.
         * septentrio_gnss_driver::PVTGeodetic, or nmea_msgs::GPGGA. Note that
         * "typename" could be omitted in the argument.
         * @param message_key The pair's key
         * @return The modified multimap "callbackmap_"
         */
        template <typename T>
        CallbackMap insert(std::string message_key)
        {
            boost::mutex::scoped_lock lock(callback_mutex_);
            // Adding typename might be cleaner, but is optional again
            CallbackHandler<T>* handler = new CallbackHandler<T>();
            callbackmap_.insert(std::make_pair(
                message_key, boost::shared_ptr<AbstractCallbackHandler>(handler)));
            CallbackMap::key_type key = message_key;
            ROS_DEBUG("Key %s successfully inserted into multimap: %s",
                      message_key.c_str(),
                      ((unsigned int)callbackmap_.count(key)) ? "true" : "false");
            return callbackmap_;
        }

        /**
         * @brief Ćalled every time rx_message is found to contain some potentially
         * useful message
         * @param rx_message
         */
        void handle(RxMessage& rx_message);

        /**
         * @brief Searches for Rx messages that could potentially be
         * decoded/parsed/published
         * @param[in] data Buffer passed on from AsyncManager class
         * @param[in] size Size of the buffer
         */
        void readCallback(const uint8_t* data, std::size_t& size);

        //! Callback handlers multimap for Rx messages; it needs to be public since
        //! we copy-assign (did not work otherwise) new callbackmap_, after inserting
        //! a pair to the multimap within the DefineMessages() method of the
        //! ROSaicNode class, onto its old version.
        CallbackMap callbackmap_;

    private:
        //! The "static" keyword resolves construct-by-copying issues related to this
        //! mutex by making it available throughout the code unit. The mutex
        //! constructor list contains "mutex (const mutex&) = delete", hence
        //! construct-by-copying a mutex is explicitly prohibited. The get_handlers()
        //! method of the Comm_IO class hence forces us to make this mutex static.
        static boost::mutex callback_mutex_;

        //! Determines which of the SBF blocks necessary for the gps_common::GPSFix
        //! ROS message arrives last and thus launches its construction
        static std::string do_gpsfix_;

        //! Determines which of the SBF blocks necessary for the
        //! sensor_msgs::NavSatFix ROS message arrives last and thus launches its
        //! construction
        static std::string do_navsatfix_;

        //! Determines which of the SBF blocks necessary for the
        //! geometry_msgs/PoseWithCovarianceStamped ROS message arrives last and thus
        //! launches its construction
        static std::string do_pose_;

        //! Determines which of the SBF blocks necessary for the
        //! diagnostic_msgs/DiagnosticArray ROS message arrives last and thus
        //! launches its construction
        static std::string do_diagnostics_;

        //! Shorthand for the map responsible for matching ROS message identifiers
        //! relevant for GPSFix to a uint32_t
        typedef std::map<std::string, uint32_t> GPSFixMap;

        //! All instances of the CallbackHandlers class shall have access to the map
        //! without reinitializing it, hence static
        static GPSFixMap gpsfix_map;

        //! Shorthand for the map responsible for matching ROS message identifiers
        //! relevant for NavSatFix to a uint32_t
        typedef std::map<std::string, uint32_t> NavSatFixMap;

        //! All instances of the CallbackHandlers class shall have access to the map
        //! without reinitializing it, hence static
        static NavSatFixMap navsatfix_map;

        //! Shorthand for the map responsible for matching ROS message identifiers
        //! relevant for PoseWithCovarianceStamped to a uint32_t
        typedef std::map<std::string, uint32_t> PoseWithCovarianceStampedMap;

        //! All instances of the CallbackHandlers class shall have access to the map
        //! without reinitializing it, hence static
        static PoseWithCovarianceStampedMap pose_map;

        //! Shorthand for the map responsible for matching ROS message identifiers
        //! relevant for DiagnosticArray to a uint32_t
        typedef std::map<std::string, uint32_t> DiagnosticArrayMap;

        //! All instances of the CallbackHandlers class shall have access to the map
        //! without reinitializing it, hence static
        static DiagnosticArrayMap diagnosticarray_map;
    };

} // namespace io_comm_rx

#endif
