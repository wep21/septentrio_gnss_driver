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

//! 0x24 is ASCII for $ - 1st byte in each message
#ifndef SBF_SYNC_BYTE_1
#define SBF_SYNC_BYTE_1 0x24
#endif
//! 0x40 is ASCII for @ - 2nd byte to indicate SBF block
#ifndef SBF_SYNC_BYTE_2
#define SBF_SYNC_BYTE_2 0x40
#endif
//! 0x24 is ASCII for $ - 1st byte in each message
#ifndef NMEA_SYNC_BYTE_1
#define NMEA_SYNC_BYTE_1 0x24
#endif
//! 0x47 is ASCII for G - 2nd byte to indicate NMEA-type ASCII message
#ifndef NMEA_SYNC_BYTE_2_1
#define NMEA_SYNC_BYTE_2_1 0x47
#endif
//! 0x50 is ASCII for P - 2nd byte to indicate proprietary ASCII message
#ifndef NMEA_SYNC_BYTE_2_2
#define NMEA_SYNC_BYTE_2_2 0x50
#endif
//! 0x24 is ASCII for $ - 1st byte in each response from the Rx
#ifndef RESPONSE_SYNC_BYTE_1
#define RESPONSE_SYNC_BYTE_1 0x24
#endif
//! 0x52 is ASCII for R (for "Response") - 2nd byte in each response from the Rx
#ifndef RESPONSE_SYNC_BYTE_2
#define RESPONSE_SYNC_BYTE_2 0x52
#endif
//! 0x0D is ASCII for "Carriage Return", i.e. "Enter"
#ifndef CARRIAGE_RETURN
#define CARRIAGE_RETURN 0x0D
#endif
//! 0x0A is ASCII for "Line Feed", i.e. "New Line"
#ifndef LINE_FEED
#define LINE_FEED 0x0A
#endif
//! 0x3F is ASCII for ? - 3rd byte in the response message from the Rx in case the
//! command was invalid
#ifndef RESPONSE_SYNC_BYTE_3
#define RESPONSE_SYNC_BYTE_3 0x3F
#endif
//! 0x49 is ASCII for I - 1st character of connection descriptor sent by the Rx after
//! initiating TCP connection
#ifndef CONNECTION_DESCRIPTOR_BYTE_1
#define CONNECTION_DESCRIPTOR_BYTE_1 0x49
#endif
//! 0x50 is ASCII for P - 2nd character of connection descriptor sent by the Rx after
//! initiating TCP connection
#ifndef CONNECTION_DESCRIPTOR_BYTE_2
#define CONNECTION_DESCRIPTOR_BYTE_2 0x50
#endif

// C++ libraries
#include <cassert> // for assert
#include <cstddef>
#include <map>
#include <sstream>
// Boost includes
#include <boost/call_traits.hpp>
#include <boost/format.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/tokenizer.hpp>
// ROS includes
#include <diagnostic_msgs/DiagnosticArray.h>
#include <diagnostic_msgs/DiagnosticStatus.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <gps_common/GPSFix.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/TimeReference.h>
// ROSaic includes
#include <septentrio_gnss_driver/AttCovEuler.h>
#include <septentrio_gnss_driver/AttEuler.h>
#include <septentrio_gnss_driver/PVTCartesian.h>
#include <septentrio_gnss_driver/PVTGeodetic.h>
#include <septentrio_gnss_driver/PosCovCartesian.h>
#include <septentrio_gnss_driver/PosCovGeodetic.h>
// INS include
#include <septentrio_gnss_driver/INSNavCart.h>
#include <septentrio_gnss_driver/INSNavGeod.h>

#include <septentrio_gnss_driver/crc/crc.h>
#include <septentrio_gnss_driver/parsers/nmea_parsers/gpgga.hpp>
#include <septentrio_gnss_driver/parsers/nmea_parsers/gpgsa.hpp>
#include <septentrio_gnss_driver/parsers/nmea_parsers/gpgsv.hpp>
#include <septentrio_gnss_driver/parsers/nmea_parsers/gprmc.hpp>
#include <septentrio_gnss_driver/parsers/string_utilities.h>

#ifndef RX_MESSAGE_HPP
#define RX_MESSAGE_HPP

/**
 * @file rx_message.hpp
 * @date 20/08/20
 * @brief Defines a class that reads messages handed over from the circular buffer
 */

extern bool g_use_gnss_time;
extern bool g_read_cd;
extern uint32_t g_cd_count;
extern uint32_t g_leap_seconds;
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
extern boost::shared_ptr<ros::NodeHandle> g_nh;
extern const uint32_t g_ROS_QUEUE_SIZE;
extern ros::Time g_unix_time;
extern bool g_read_from_sbf_log;
extern bool g_read_from_pcap;

//! Enum for NavSatFix's status.status field, which is obtained from PVTGeodetic's
//! Mode field
enum TypeOfPVT_Enum
{
    evNoPVT,
    evStandAlone,
    evDGPS,
    evFixed,
    evRTKFixed,
    evRTKFloat,
    evSBAS,
    evMovingBaseRTKFixed,
    evMovingBaseRTKFloat,
    evPPP
};

//! Since switch only works with int (yet NMEA message IDs are strings), we need
//! enum. Note drawbacks: No variable can have a name which is already in some
//! enumeration, enums are not type safe etc..
enum RxID_Enum
{
    evNavSatFix,
    evGPSFix,
    evPoseWithCovarianceStamped,
    evGPGGA,
    evGPRMC,
    evGPGSA,
    evGPGSV,
    evGLGSV,
    evGAGSV,
    evPVTCartesian,
    evPVTGeodetic,
    evPosCovCartesian,
    evPosCovGeodetic,
    evAttEuler,
    evAttCovEuler,
    evINSNavCart,
    evINSNavGeod,
    evGPST,
    evChannelStatus,
    evMeasEpoch,
    evDOP,
    evVelCovGeodetic,
    evDiagnosticArray,
    evReceiverStatus,
    evQualityInd,
    evReceiverSetup
};

namespace io_comm_rx {
    /**
     * @brief Calculates the timestamp, in the Unix Epoch time format
     * This is either done using the TOW as transmitted with the SBF block (if
     * "use_gnss" is true), or using the current time.
     * @param[in] tow (Time of Week) Number of milliseconds that elapsed since the
     * beginning of the current GPS week as transmitted by the SBF block
     * @param[in] use_gnss If true, the TOW as transmitted with the SBF block is
     * used, otherwise the current time
     * @return ros::Time object containing seconds and nanoseconds since last epoch
     */
    ros::Time timestampSBF(uint32_t tow, bool use_gnss);

    /**
     * @class RxMessage
     * @brief Can search buffer for messages, read/parse them, and so on
     */
    class RxMessage
    {
    public:
        /**
         * @brief Constructor of the RxMessage class
         *
         * One can always provide a non-const value where a const one was expected.
         * The const-ness of the argument just means the function promises not to
         * change it.. Recall: static_cast by the way can remove or add const-ness,
         * no other C++ cast is capable of removing it (not even reinterpret_cast)
         * @param[in] data Pointer to the buffer that is about to be analyzed
         * @param[in] size Size of the buffer (as handed over by async_read_some)
         */
        RxMessage(const uint8_t* data, std::size_t& size) : data_(data), count_(size)
        {
            found_ = false;
            crc_check_ = false;
            message_size_ = 0;
        }

        //! Determines whether data_ points to the SBF block with ID "ID", e.g. 5003
        bool isMessage(const uint16_t ID);
        //! Determines whether data_ points to the NMEA message with ID "ID", e.g.
        //! "$GPGGA"
        bool isMessage(std::string ID);
        //! Determines whether data_ currently points to an SBF block
        bool isSBF();
        //! Determines whether data_ currently points to an NMEA message
        bool isNMEA();
        //! Determines whether data_ currently points to an NMEA message
        bool isResponse();
        //! Determines whether data_ currently points to a connection descriptor
        //! (right after initiating a TCP connection)
        bool isConnectionDescriptor();
        //! Determines whether data_ currently points to an error message reply from
        //! the Rx
        bool isErrorMessage();
        //! Determines size of the message (also command reply) that data_ is
        //! currently pointing at
        std::size_t messageSize();
        //! Returns the message ID of the message where data_ is pointing at at the
        //! moment, SBF identifiers embellished with inverted commas, e.g. "5003"
        std::string messageID();

        /**
         * @brief Returns the count_ variable
         * @return The variable count_
         */
        std::size_t getCount() { return count_; };
        /**
         * @brief Searches the buffer for the beginning of the next message (NMEA or
         * SBF)
         * @return A pointer to the start of the next message.
         */
        const uint8_t* search();

        /**
         * @brief Gets the length of the SBF block
         *
         * It determines the length from the header of the SBF block. The block
         * length thus includes the header length.
         * @return The length of the SBF block
         */
        uint16_t getBlockLength();

        /**
         * @brief Gets the current position in the read buffer
         * @return The current position of the read buffer
         */
        const uint8_t* getPosBuffer();

        /**
         * @brief Gets the end position in the read buffer
         * @return A pointer pointing to just after the read buffer (matches
         * search()'s final pointer in case no valid message is found)
         */
        const uint8_t* getEndBuffer();

        /**
         * @brief Has an NMEA message, SBF block or command reply been found in the
         * buffer?
         * @returns True if a message with the correct header & length has been found
         */
        bool found();

        /**
         * @brief Goes to the start of the next message based on the calculated
         * length of current message
         */
        void next();

        /**
         * @brief Performs the CRC check (if SBF) and publishes ROS messages
         * @return True if read was successful, false otherwise
         */
        bool read(std::string message_key, bool search = false);

        /**
         * @brief Whether or not a message has been found
         */
        bool found_;

    private:
        /**
         * @brief Pointer to the buffer of messages
         */
        const uint8_t* data_;

        /**
         * @brief Number of unread bytes in the buffer
         */
        std::size_t count_;

        /**
         * @brief Whether the CRC check as evaluated in the read() method was
         * successful or not is stored here
         */
        bool crc_check_;

        /**
         * @brief Helps to determine size of response message / NMEA message / SBF
         * block
         */
        std::size_t message_size_;

        /**
         * @brief Number of times the gps_common::GPSFix message has been published
         */
        static uint32_t count_gpsfix_;

        /**
         * @brief Since NavSatFix etc. need PVTGeodetic, incoming PVTGeodetic blocks
         * need to be stored
         */
        static PVTGeodetic last_pvtgeodetic_;

        /**
         * @brief Since NavSatFix etc. need PosCovGeodetic, incoming PosCovGeodetic
         * blocks need to be stored
         */
        static PosCovGeodetic last_poscovgeodetic_;

        /**
         * @brief Since GPSFix etc. need AttEuler, incoming AttEuler blocks need to
         * be stored
         */
        static AttEuler last_atteuler_;

        /**
         * @brief Since GPSFix etc. need AttCovEuler, incoming AttCovEuler blocks
         * need to be stored
         */
        static AttCovEuler last_attcoveuler_;

        /**
         * @brief Since NavSatFix etc. need INSNavGeod, incoming INSNavGeod blocks
         * need to be stored
         */
        static INSNavGeod last_insnavgeod_;

        /**
         * @brief Since GPSFix needs ChannelStatus, incoming ChannelStatus blocks
         * need to be stored
         */
        static ChannelStatus last_channelstatus_;

        /**
         * @brief Since GPSFix needs MeasEpoch (for SNRs), incoming MeasEpoch blocks
         * need to be stored
         */
        static MeasEpoch last_measepoch_;

        /**
         * @brief Since GPSFix needs DOP, incoming DOP blocks need to be stored
         */
        static DOP last_dop_;

        /**
         * @brief Since GPSFix needs VelCovGeodetic, incoming VelCovGeodetic blocks
         * need to be stored
         */
        static VelCovGeodetic last_velcovgeodetic_;

        /**
         * @brief Since DiagnosticArray needs ReceiverStatus, incoming ReceiverStatus
         * blocks need to be stored
         */
        static ReceiverStatus last_receiverstatus_;

        /**
         * @brief Since DiagnosticArray needs QualityInd, incoming QualityInd blocks
         * need to be stored
         */
        static QualityInd last_qualityind_;

        /**
         * @brief Since DiagnosticArray needs ReceiverSetup, incoming ReceiverSetup
         * blocks need to be stored
         */
        static ReceiverSetup last_receiversetup_;
        
        //! Shorthand for the map responsible for matching PVTGeodetic's Mode field
        //! to an enum value
        typedef std::map<uint16_t, TypeOfPVT_Enum> TypeOfPVTMap;

        /**
         * @brief All instances of the RxMessage class shall have access to the map
         * without reinitializing it, hence static
         */
        static TypeOfPVTMap type_of_pvt_map;

        //! Shorthand for the map responsible for matching ROS message identifiers to
        //! an enum value
        typedef std::map<std::string, RxID_Enum> RxIDMap;

        /**
         * @brief All instances of the RxMessage class shall have access to the map
         * without reinitializing it, hence static
         *
         * This map is for mapping ROS message, SBF and NMEA identifiers to an
         * enumeration and makes the switch-case formalism in rx_message.hpp more
         * explicit.
         */
        static RxIDMap rx_id_map;

        /**
         * @brief Callback function when reading PVTCartesian blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message PVTCartesian
         * @return A smart pointer to the ROS message PVTCartesian just created
         */
        septentrio_gnss_driver::PVTCartesianPtr
        PVTCartesianCallback(PVTCartesian& data);

        /**
         * @brief Callback function when reading PVTGeodetic blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message PVTGeodetic
         * @return A smart pointer to the ROS message PVTGeodetic just created
         */
        septentrio_gnss_driver::PVTGeodeticPtr
        PVTGeodeticCallback(PVTGeodetic& data);

        /**
         * @brief Callback function when reading PosCovCartesian blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message PosCovCartesian
         * @return A smart pointer to the ROS message PosCovCartesian just created
         */
        septentrio_gnss_driver::PosCovCartesianPtr
        PosCovCartesianCallback(PosCovCartesian& data);

        /**
         * @brief Callback function when reading PosCovGeodetic blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message PosCovGeodetic
         * @return A smart pointer to the ROS message PosCovGeodetic just created
         */
        septentrio_gnss_driver::PosCovGeodeticPtr
        PosCovGeodeticCallback(PosCovGeodetic& data);

        /**
         * @brief Callback function when reading AttEuler blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message AttEuler
         * @return A smart pointer to the ROS message AttEuler just created
         */
        septentrio_gnss_driver::AttEulerPtr AttEulerCallback(AttEuler& data);

        /**
         * @brief Callback function when reading AttCovEuler blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message AttCovEuler
         * @return A smart pointer to the ROS message AttCovEuler just created
         */
        septentrio_gnss_driver::AttCovEulerPtr
        AttCovEulerCallback(AttCovEuler& data);

        /**
         * @brief Callback function when reading INSNavCart blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message INSNavCart
         * @return A smart pointer to the ROS message INSNavCart just created
         */
        septentrio_gnss_driver::INSNavCartPtr
        INSNavCartCallback(INSNavCart& data);

        /**
         * @brief Callback function when reading INSNavGeod blocks
         * @param[in] data The (packed and aligned) struct instance used to populate
         * the ROS message INSNavGeod
         * @return A smart pointer to the ROS message INSNavGeod just created
         */
        septentrio_gnss_driver::INSNavGeodPtr
        INSNavGeodCallback(INSNavGeod& data);

        /**
         * @brief "Callback" function when constructing NavSatFix messages
         * @return A smart pointer to the ROS message NavSatFix just created
         */
        sensor_msgs::NavSatFixPtr NavSatFixCallback();

        /**
         * @brief "Callback" function when constructing GPSFix messages
         * @return A smart pointer to the ROS message GPSFix just created
         */
        gps_common::GPSFixPtr GPSFixCallback();

        /**
         * @brief "Callback" function when constructing PoseWithCovarianceStamped
         * messages
         * @return A smart pointer to the ROS message PoseWithCovarianceStamped just
         * created
         */
        geometry_msgs::PoseWithCovarianceStampedPtr
        PoseWithCovarianceStampedCallback();

        /**
         * @brief "Callback" function when constructing
         * diagnostic_msgs::DiagnosticArray messages
         * @return A smart pointer to the ROS message
         * diagnostic_msgs::DiagnosticArray just created
         */
        diagnostic_msgs::DiagnosticArrayPtr DiagnosticArrayCallback();
    };
} // namespace io_comm_rx
#endif // for RX_MESSAGE_HPP
