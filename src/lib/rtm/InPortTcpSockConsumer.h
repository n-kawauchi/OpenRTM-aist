// -*- C++ -*-
/*!
 * @file  InPortTcpSockConsumer.h
 * @brief InPortTcpSockConsumer class
 * @date  $Date: 2008-01-13 07:41:50 $
 * @author Noriaki Ando <n-ando@aist.go.jp>
 *
 * Copyright (C) 2007-2008
 *     Noriaki Ando
 *     Task-intelligence Research Group,
 *     Intelligent Systems Research Institute,
 *     National Institute of
 *         Advanced Industrial Science and Technology (AIST), Japan
 *     All rights reserved.
 *
 * $Id$
 *
 */

#ifndef RTC_INPORTTCPSOCKCONSUMER_H
#define RTC_INPORTTCPSOCKCONSUMER_H

#include <rtm/BufferBase.h>
#include <rtm/InPortConsumer.h>
#include <rtm/StringUtil.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <iostream>

namespace RTC
{
  /*!
   * @if jp
   *
   * @class InPortTcpSockConsumer
   *
   * @brief InPortTcpSockConsumer クラス
   *
   * 通信手段に TCP ソケット を利用した入力ポートコンシューマの実装クラス。
   *
   * @param DataType 当該コンシューマに割り当てたバッファが保持するデータ型
   *
   * @since 0.4.1
   *
   * @else
   * @class InPortTcpSockConsumer
   *
   * @brief InPortTcpSockConsumer class
   *
   * This is an implementation class of OutPort Consumer 
   * that uses TCP socket for means of communication.
   *
   * @param DataType Data type held by the buffer that attached to this provider
   *
   * @since 0.4.1
   *
   * @endif
   */
  template <class DataType>
  class InPortTcpSockConsumer
    : public InPortConsumer
  {
  public:
    /*!
     * @if jp
     * @brief コンストラクタ
     *
     * コンストラクタ
     *
     * @param buffer 当該コンシューマに割り当てるバッファオブジェクト
     * @param prop TCP 通信設定用プロパティ
     *
     * @else
     * @brief Constructor
     *
     * Constructor
     *
     * @param buffer Buffer objects that are attached to this Consumer
     * @param prop Properties for TCP communication setup
     *
     * @endif
     */
    InPortTcpSockConsumer(BufferBase<DataType>& buffer, Properties& prop)
      : m_buffer(buffer), m_prop(prop), m_byteswap(true)
    {
      m_byteswap = toBool(m_prop["byteswap"], "YES", "NO", true);
    }
    
    /*!
     * @if jp
     * @brief コピーコンストラクタ
     *
     * コピーコンストラクタ
     *
     * @param consumer コピー元 InPortTcpSockConsumer オブジェクト
     *
     * @else
     * @brief Copy constructor
     *
     * Copy constructor
     *
     * @param consumer InPortTcpSockConsumer object of copy source
     *
     * @endif
     */
    InPortTcpSockConsumer(const InPortTcpSockConsumer<DataType>& consumer)
      : m_buffer(consumer.m_buffer), m_prop(consumer.m_prop),
	m_byteswap(consumer.m_byteswap), m_stream(consumer.m_stream),
	m_addr(consumer.m_addr) 
    {
    }
    
    /*!
     * @if jp
     * @brief 代入演算子
     *
     * 代入演算子
     *
     * @param consumer 代入元 InPortTcpSockConsumer オブジェクト
     *
     * @return 代入結果
     *
     * @else
     * @brief Assignment operator
     *
     * Assignment operator
     *
     * @param consumer InPortTcpSockConsumer object of assignment source
     *
     * @return The assignment result
     *
     * @endif
     */
    InPortTcpSockConsumer&
    operator=(const InPortTcpSockConsumer<DataType>& consumer)
    {
      m_buffer = consumer.m_buffer;
      m_prop   = consumer.m_prop;
      m_stream = consumer.m_stream;
      m_addr   = consumer.m_addr;
      m_byteswap = consumer.m_byteswap;
    }
    
    /*!
     * @if jp
     * @brief デストラクタ
     *
     * デストラクタ
     *
     * @else
     * @brief Destructor
     *
     * Destructor
     *
     * @endif
     */
    virtual ~InPortTcpSockConsumer(void)
    {}
    
    /*!
     * @if jp
     * @brief バッファからのデータ取出
     *
     * バッファからデータを取り出して送出する。
     *
     * @else
     * @brief Read data from the buffer
     *
     * Read data from the buffer and send it.
     *
     * @endif
     */
    void push()
    {
      // read data from buffer
      DataType data;
      m_buffer.read(data);
      std::cout << "byteswap: " << m_byteswap << std::endl;
      
      CORBA::ULong size(0);
      cdrMemoryStream cdrData;
      cdrData.setByteSwapFlag(m_byteswap);
      size >>= cdrData;
      data >>= cdrData;
      
      cdrMemoryStream cdrSize;
      size = cdrData.bufSize();
      cdrSize.setByteSwapFlag(m_byteswap);
      size >>= cdrSize;
      
      // hmm... type-unsafe way.
      memcpy(cdrData.bufPtr(), cdrSize.bufPtr(), sizeof(CORBA::ULong));
      
      int send(0);
      send = m_stream->send_n(cdrData.bufPtr(), size);
      
      if (send != (int)size)
	{
	  std::cout << "data size: " << size << std::endl;
	  std::cout << "send size: " << send << std::endl;
	  std::cout << "invalid send" << std::endl;
	}
    }
    
    /*!
     * @if jp
     * @brief コピーの生成
     *
     * 当該InPortTcpSockConsumerオブジェクトの複製を生成する。
     *
     * @return コピーされたInPortTcpSockConsumerオブジェクト
     *
     * @else
     * @brief Create clone
     *
     * Create clone objects of this InPortTcpSockConsumer.
     *
     * @return Clone InPortTcpSockConsumer objects
     *
     * @endif
     */
    virtual InPortTcpSockConsumer* clone() const
    {
      return new InPortTcpSockConsumer<DataType>(*this);
    }
    
    /*!
     * @if jp
     * @brief データ送信通知への登録
     *
     * 指定されたプロパティに基づいて、データ送出通知の受け取りに登録する。
     *
     * @param properties 登録情報
     *
     * @return 登録処理結果(登録成功:true、登録失敗:false)
     *
     * @else
     * @brief Subscribe to the data send notification
     *
     * Subscribe the data send notification according to the specified property
     * information.
     *
     * @param properties Information for subscription
     *
     * @return Subscription result (Successful:true, Failed:false)
     *
     * @endif
     */
    virtual bool subscribeInterface(const SDOPackage::NVList& properties)
    {
      if (!NVUtil::isStringValue(properties,
				 "dataport.dataflow_type",
				 "Push"))
	{
	  return false;
	}
      
      CORBA::Long index;
      index = NVUtil::find_index(properties,
				 "dataport.tcp_any.inport_addr");
      if (index < 0)
	{
	  return false;
	}
      
      const char* inport_addr;
      if (!(properties[index].value >>= inport_addr))
	{
	  return false;
	}
      
      std::vector<std::string> addr(split(inport_addr, ":"));
      
      if (addr.size() != 2)
	{
	  return false;
	}
      
      m_addr.set(addr[1].c_str(), addr[0].c_str());
      std::cout << "addr: " << addr[0].c_str() << std::endl;
      std::cout << "port: " << addr[1].c_str() << std::endl;
      m_stream = new ACE_SOCK_Stream();
      if (m_connector.connect(*m_stream, m_addr) == -1)
	{
	  std::cout << "Connection failed." << std::endl;
	  return false;
	}
      
      // Disable Nagle's algorithm for small packaets
      int flag(1);
      m_stream->set_option(IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
      
      return true;
    }
    
    /*!
     * @if jp
     * @brief データ送信通知からの登録解除
     *
     * データ送出通知の受け取りから登録を解除し、TCP ストリームを閉じる。
     *
     * @else
     * @brief Unsubscribe the data send notification
     *
     * Unsubscribe the data send notification and close the TCP stream.
     *
     * @endif
     */
    virtual void unsubscribeInterface(const SDOPackage::NVList& properties)
    {
      std::cout << "unsubscribe" << std::endl;
      m_stream->close();
    }
    
  private:
    BufferBase<DataType>& m_buffer;
    Properties& m_prop;
    bool m_byteswap;
    
    ACE_SOCK_Stream* m_stream;
    ACE_SOCK_Connector m_connector;
    ACE_INET_Addr m_addr;
  };
};     // namespace RTC
#endif // RTC_INPORTTCPSOCKCONSUMER_H
