/****************************************************************************************************************************
  message.hpp
  For WebSockets2_Generic Library
  
  Based on and modified from Gil Maimon's ArduinoWebsockets library https://github.com/gilmaimon/ArduinoWebsockets
  to support nRF52 and SAMD21/SAMD51 boards besides ESP8266 and ESP32

  The library provides simple and easy interface for websockets (Client and Server).
  
  Built by Khoi Hoang https://github.com/khoih-prog/Websockets2_Generic
  Licensed under MIT license
  Version: 1.0.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      14/07/2020 Initial coding/porting to support nRF52 and SAMD21/SAMD51 boards. Add SINRIC/Alexa support      
 *****************************************************************************************************************************/
 
#pragma once

#include <Tiny_Websockets_Generic/internals/ws_common.hpp>
#include <Tiny_Websockets_Generic/internals/data_frame.hpp>

namespace websockets2_generic
{
  enum class MessageType
  {
    Empty,
    Text, Binary,
    Ping, Pong, Close
  };
  
  MessageType messageTypeFromOpcode(uint8_t opcode);
  
  enum class MessageRole
  {
    Complete, First, Continuation, Last
  };
  
  // The class the user will interact with as a message
  // This message can be partial (so practically this is a Frame and not a message)
  struct WebsocketsMessage
  {
    WebsocketsMessage(MessageType msgType, const WSString& msgData, MessageRole msgRole = MessageRole::Complete) : _type(msgType), _length(msgData.size()), _data(msgData), _role(msgRole) {}
    WebsocketsMessage() : WebsocketsMessage(MessageType::Empty, "", MessageRole::Complete) {}

    static WebsocketsMessage CreateFromFrame(internals2_generic::WebsocketsFrame frame, MessageType overrideType = MessageType::Empty) 
    {
      auto type = overrideType;
      
      if (type == MessageType::Empty)
      {
        type = messageTypeFromOpcode(frame.opcode);
      }

      // deduce role
      MessageRole msgRole = MessageRole::Complete;
      if (frame.isNormalUnfragmentedMessage()) 
      {
        msgRole = MessageRole::Complete;
      } 
      else if (frame.isBeginningOfFragmentsStream()) 
      {
        msgRole = MessageRole::First;
      } 
      else if (frame.isContinuesFragment()) 
      {
        msgRole = MessageRole::Continuation;
      } 
      else if (frame.isEndOfFragmentsStream()) 
      {
        msgRole = MessageRole::Last;
      }

      return WebsocketsMessage(type, std::move(frame.payload), msgRole);
    }

    // for validation
    bool isEmpty() const 
    {
      return this->_type == MessageType::Empty;
    }

    // Type Helper Functions
    MessageType type() const 
    {
      return this->_type;
    }

    bool isText() const 
    {
      return this->_type == MessageType::Text;
    }
    
    bool isBinary() const 
    {
      return this->_type == MessageType::Binary;
    }

    bool isPing() const 
    {
      return this->_type == MessageType::Ping;
    }
    
    bool isPong() const 
    {
      return this->_type == MessageType::Pong;
    }

    bool isClose() const 
    {
      return this->_type == MessageType::Close;
    }

    // Role Helper Function
    MessageRole role() const 
    {
      return this->_role;
    }

    bool isComplete() const 
    {
      return this->_role == MessageRole::Complete;
    }
    
    bool isPartial() const 
    {
      return this->_role != MessageRole::Complete;
    }
    
    bool isFirst() const 
    {
      return this->_role == MessageRole::First;
    }
    
    bool isContinuation() const 
    {
      return this->_role == MessageRole::Continuation;
    }
    
    bool isLast() const 
    {
      return this->_role == MessageRole::Last;
    }

    WSInterfaceString data() const 
    {
      return internals2_generic::fromInternalString(this->_data);
    }
    
    const WSString& rawData() const 
    {
      return this->_data;
    }
    
    const char* c_str() const 
    {
      return this->_data.c_str();
    }

    uint32_t length() const 
    {
      return this->_length;
    }

    class StreamBuilder 
    {
      public:
        StreamBuilder(bool dummyMode = false) : _dummyMode(dummyMode), _empty(true) {}

        void first(const internals2_generic::WebsocketsFrame& frame) 
        {
          if (this->_empty == false) 
          {
            badFragment();
            return;
          }

          this->_empty = false;
          
          if (frame.isBeginningOfFragmentsStream()) 
          {
            this->_isComplete = false;
            this->_didErrored = false;

            if (this->_dummyMode == false) 
            {
              this->_content = std::move(frame.payload);
            }

            this->_type = messageTypeFromOpcode(frame.opcode);
            
            if (this->_type == MessageType::Empty) 
            {
              badFragment();
            }
          } 
          else 
          {
            this->_didErrored = true;
          }
        }

        void append(const internals2_generic::WebsocketsFrame& frame) 
        {
          if (isErrored()) 
            return;
            
          if (isEmpty() || isComplete()) 
          {
            badFragment();
            return;
          }

          if (frame.isContinuesFragment()) 
          {
            if (this->_dummyMode == false) 
            {
              this->_content += std::move(frame.payload);
            }
          } 
          else 
          {
            badFragment();
          }
        }

        void end(const internals2_generic::WebsocketsFrame& frame) 
        {
          if (isErrored()) 
            return;
            
          if (isEmpty() || isComplete()) 
          {
            badFragment();
            return;
          }

          if (frame.isEndOfFragmentsStream()) 
          {
            if (this->_dummyMode == false) 
            {
              this->_content += std::move(frame.payload);
            }
            
            this->_isComplete = true;
          } 
          else 
          {
            badFragment();
          }
        }

        void badFragment() 
        {
          this->_didErrored = true;
          this->_isComplete = false;
        }

        bool isErrored() 
        {
          return this->_didErrored;
        }

        bool isOk() 
        {
          return !this->_didErrored;
        }

        bool isComplete() 
        {
          return this->_isComplete;
        }

        bool isEmpty() 
        {
          return this->_empty;
        }

        MessageType type() 
        {
          return this->_type;
        }

        WebsocketsMessage build() 
        {
          return WebsocketsMessage(
                   this->_type,
                   std::move(this->_content),
                   MessageRole::Complete
                 );
        }

      private:
        bool _dummyMode;
        bool _empty;
        bool _isComplete = false;
        WSString _content;
        MessageType _type;
        bool _didErrored;
        
    };    // class StreamBuilder 
  
    private:
      const MessageType _type;
      const uint32_t _length;
      const WSString _data;
      const MessageRole _role;
      
  };    // struct WebsocketsMessage
}       // namespace websockets2_generic
