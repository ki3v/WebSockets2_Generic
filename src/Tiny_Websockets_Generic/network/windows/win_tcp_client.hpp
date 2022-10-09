/****************************************************************************************************************************
  win_tcp_client.hpp
  For WebSockets2_Generic Library
  
  Based on and modified from Gil Maimon's ArduinoWebsockets library https://github.com/gilmaimon/ArduinoWebsockets
  to support STM32F/L/H/G/WB/MP1, nRF52, SAMD21/SAMD51, SAM DUE, Teensy, RP2040 boards besides ESP8266 and ESP32

  The library provides simple and easy interface for websockets (Client and Server).
  
  Built by Khoi Hoang https://github.com/khoih-prog/Websockets2_Generic
  Licensed under MIT license
  
  Version: 1.12.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      14/07/2020 Initial coding/porting to support nRF52 and SAMD21/SAMD51 boards. Add SINRIC/Alexa support
  ...
  1.9.0   K Hoang      30/11/2021 Auto detect ESP32 core version. Fix bug in examples
  1.9.1   K Hoang      17/12/2021 Fix QNEthernet TCP interface
  1.10.0  K Hoang      18/12/2021 Supporting case-insensitive headers, according to RFC2616
  1.10.1  K Hoang      26/02/2022 Reduce QNEthernet latency
  1.10.2  K Hoang      14/03/2022 Fix bug when using QNEthernet staticIP. Add staticIP option to NativeEthernet
  1.10.3  K Hoang      11/04/2022 Use Ethernet_Generic library as default. Support SPI1/SPI2 for RP2040
  1.11.0  K Hoang      08/10/2022 Add support to ESP32 using W5x00 Ethernet
  1.12.0  K Hoang      09/10/2022 Add support to ENC28J60 using EthernetENC or UIPEthernet for all supported boards
 *****************************************************************************************************************************/
 
#pragma once

#ifdef _WIN32 

#include <Tiny_Websockets_Generic/internals/ws_common.hpp>
#include <Tiny_Websockets_Generic/network/tcp_client.hpp>

#define WIN32_LEAN_AND_MEAN

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

namespace websockets2_generic
{
  namespace network2_generic
  {
    class WinTcpClient : public TcpClient 
    {
      public:
        WinTcpClient(const SOCKET s = INVALID_SOCKET);
        bool connect(const WSString& host, const int port) override;
        bool poll() override;
        bool available() override;
        void send(const WSString& data) override;
        void send(const WSString&& data) override;
        void send(const uint8_t* data, const uint32_t len) override;
        WSString readLine() override;
        void read(uint8_t* buffer, const uint32_t len) override;
        void close() override;
        virtual ~WinTcpClient();
    
      protected:
        int getSocket() const override;
    
      private:
        SOCKET socket;
    };
  }   // namespace network2_generic
}     // namespace websockets2_generic


#endif // #ifdef _WIN32 
