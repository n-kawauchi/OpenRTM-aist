// -*- C++ -*-
/*!
 * @file  FastDDSManager.cpp
 * @brief FastDDSManager class
 * @date  $Date: 2019-01-31 03:08:03 $
 * @author Nobuhiko Miyamoto <n-miyamoto@aist.go.jp>
 *
 * Copyright (C) 2019
 *     Nobuhiko Miyamoto
 *     Robot Innovation Research Center,
 *     National Institute of
 *         Advanced Industrial Science and Technology (AIST), Japan
 *
 *     All rights reserved.
 *
 *
 */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#pragma warning(push)
#pragma warning(disable:4819)
#endif

#include "FastDDSManager.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>




namespace RTC
{
  FastDDSManager* FastDDSManager::manager = nullptr;
  std::mutex FastDDSManager::mutex;
  std::once_flag FastDDSManager::m_once;

  /*!
   * @if jp
   * @brief コンストラクタ
   *
   * コンストラクタ
   *
   * @else
   * @brief Constructor
   *
   * Constructor
   *
   * @endif
   */
  FastDDSManager::FastDDSManager() : m_participant(nullptr)
  {
  }

  /*!
   * @if jp
   * @brief コピーコンストラクタ
   *  
   * @param manager FastDDSManager
   *
   * @else
   * @brief Copy Constructor
   *
   * @param manager FastDDSManager
   *
   * @endif
   */
  FastDDSManager::FastDDSManager(const FastDDSManager &/*mgr*/) : m_participant(nullptr)
  {
    
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
  FastDDSManager::~FastDDSManager()
  {
    
  }

  /*!
   * @if jp
   * @brief トピックマネージャ開始
   *
   *
   * @else
   * @brief 
   *
   * 
   *
   * @endif
   */
  void FastDDSManager::start(coil::Properties& prop)
  {
      Logger rtclog("FastDDSManager");
      RTC_INFO(("FastDDSManager::start()"));
      RTC_INFO_STR((prop));
      
      // DomainParticipantFactory のインスタンスを取得
      auto factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();

      m_xml_profile_file = std::move(prop["xmlprofile.filename"]);
      if (!m_xml_profile_file.empty())
      {
        // Fast DDS では factory 経由で XML プロファイルをロード
	factory->load_XML_profiles_file(m_xml_profile_file);
        RTC_INFO(("Load XMl file: %s", m_xml_profile_file.c_str()));

        const std::string paticipant_name = std::move(prop["participant.name"]);
        RTC_INFO(("Create participant: %s", paticipant_name.c_str()));
        // XML のプロファイル名を指定して Participant を生成
	// 第1引数は domain_id（デフォルト0など。XML側の設定でオーバーライドされる場合あり）
	m_participant = factory->create_participant_with_profile(0, participant_name);
      }
      else
      {
        uint32_t domain_id = 0;
        coil::stringTo<uint32_t>(domain_id, prop["domain.id"].c_str());
        RTC_INFO(("Domain ID: %d", domain_id));
        
        const std::string paticipant_name = prop.getProperty("participant.name", "participant_openrtm");
        RTC_INFO(("Participant name: %s", paticipant_name.c_str()));
        
	// QoS オブジェクトの作成とパラメータ設定
	eprosima::fastdds::dds::DomainParticipantQos pqos = eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
	pqos.name(participant_name.c_str());
       
        // セキュリティパラメータ等の追加設定（setParticipantSecParamもDomainParticipantQos受け取りに要修正）	
        setParticipantSecParam(prop, pqos);

        m_participant = factory->create_participant(domain_id, pqos);
      }
      if (m_participant == nullptr)
      {
        RTC_ERROR(("Failed to create DomainParticipant"));
      }
  }

  /*!
   * @if jp
   * @brief マネージャ終了
   *
   *
   * @else
   * @brief
   *
   *
   *
   * @endif
   */
  void FastDDSManager::shutdown()
  {
    if (m_participant != nullptr)
    {
      Logger rtclog("FastDDSManager");
      RTC_INFO(("FastDDSManager::shutdown()"));
      eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(m_participant);
      m_participant = nullptr;
    }
  }

  /*!
   * @if jp
   * @brief Participant取得
   *
   * @return Participant
   *
   * @else
   * @brief get Participant
   *
   * @return Participant
   *
   * @endif
   */
  eprosima::fastdds::dds:DomainParticipant* FastDDSManager::getParticipant()
  {
      std::lock_guard<std::mutex> guard(mutex);
      return m_participant;
  }

  /*!
   * @if jp
   * @brief 型の登録
   *
   * @param type 登録するデータのインスタンス
   * @return true：登録成功、false：登録失敗
   *
   * @else
   * @brief
   *
   * @param type
   * @return
   *
   * @endif
   */
  bool FastDDSManager::registerType(eprosima::fastrtps::TopicDataType* type)
  {
      std::lock_guard<std::mutex> guard(mutex);
      if (registeredType(type->getName()))
      {
          return true;
      }
      return eprosima::fastrtps::Domain::registerType(m_participant,type);
  }

  /*!
   * @if jp
   * @brief 指定名の型が登録済みかを判定する
   *
   * @param name 型名
   * @return true：登録済み、false：未登録
   *
   * @else
   * @brief
   *
   * @param name
   * @return
   *
   * @endif
   */
  bool FastDDSManager::registeredType(const char* name)
  {
      eprosima::fastrtps::TopicDataType* type_;
      return eprosima::fastrtps::Domain::getRegisteredType(m_participant, name, &type_);
  }

  /*!
   * @if jp
   * @brief 型の登録解除
   *
   * @param name 型名
   * @return true：解除成功、false：解除失敗
   *
   * @else
   * @brief
   *
   * @param name
   * @return
   *
   * @endif
   */
  bool FastDDSManager::unregisterType(const char* name)
  {
      std::lock_guard<std::mutex> guard(mutex);
      if (!registeredType(name))
      {
          return false;
      }
      return eprosima::fastrtps::Domain::unregisterType(m_participant, name);
  }

  /*!
   * @if jp
   * @brief 初期化関数
   * 
   * @return インスタンス
   *
   * @else
   * @brief 
   *
   * @return 
   * 
   *
   * @endif
   */
  FastDDSManager* FastDDSManager::init(coil::Properties& prop)
  {
    std::call_once(m_once, [&] {
      manager = new FastDDSManager();
      manager->start(prop);
      });
    return manager;
  }

  /*!
   * @if jp
   * @brief インスタンス取得
   * 
   * @return インスタンス
   *
   * @else
   * @brief 
   *
   * @return 
   * 
   *
   * @endif
   */
  FastDDSManager& FastDDSManager::instance()
  {
    std::call_once(m_once, [&] {
      coil::Properties prop;
      manager = new FastDDSManager();
      manager->start(prop);
      });
    return *manager;
  }

  /*!
   * @if jp
   * @brief FastDDSManagerが初期化されている場合に終了処理を呼び出す
   *
   *
   * @else
   * @brief
   *
   * @return
   *
   *
   * @endif
   */
  void FastDDSManager::shutdown_global()
  {
      std::lock_guard<std::mutex> guard(mutex);
      if (manager)
      {
          manager->shutdown();
      }
  }


  /*!
   * @if jp
   * @brief プロパティからeprosima::fastrtps::ParticipantAttributesを設定する
   *
   * @param prop プロパティ
   * @param PParam Participantの属性
   *
   * @else
   * @brief
   *
   * @param prop
   * @param PParam
   *
   *
   * @endif
   */
  void FastDDSManager::setParticipantSecParam(coil::Properties& prop, eprosima::fastrtps::ParticipantAttributes &PParam)
  {
    std::string auth_plugin{ std::move(prop["dds.sec.auth.plugin"]) };
    if (!auth_plugin.empty())
    {
      PParam.rtps.properties.properties().emplace_back("dds.sec.auth.plugin", auth_plugin);
      const std::string path = "dds.sec.auth." + auth_plugin;
      const std::vector<Properties*>& auth_leaf = prop.getNode(path).getLeaf();
      for (auto& lprop : auth_leaf)
      {
        std::string key(path + "." + lprop->getName()), value(lprop->getValue());
        PParam.rtps.properties.properties().emplace_back(key, value);
      }
    }

    std::string access_plugin{ std::move(prop["dds.sec.access.plugin"]) };
    if (!access_plugin.empty())
    {
      PParam.rtps.properties.properties().emplace_back("dds.sec.access.plugin", access_plugin);
      const std::string path = "dds.sec.access." + access_plugin;
      const std::vector<Properties*>& access_leaf = prop.getNode(path).getLeaf();
      for (auto& lprop : access_leaf)
      {
        std::string key(path + "." + lprop->getName()), value(lprop->getValue());
        PParam.rtps.properties.properties().emplace_back(key, value);
      }
    }

    std::string crypto_plugin{ std::move(prop["dds.sec.crypto.plugin"]) };
    if (!crypto_plugin.empty())
    {
      PParam.rtps.properties.properties().emplace_back("dds.sec.crypto.plugin", crypto_plugin);
      const std::string path = "dds.sec.crypto." + crypto_plugin;
      const std::vector<Properties*>& crypto_leaf = prop.getNode(path).getLeaf();
      for (auto& lprop : crypto_leaf)
      {
        std::string key(path + "." + lprop->getName()), value(lprop->getValue());
        PParam.rtps.properties.properties().emplace_back(key, value);
      }
    }

    std::string log_plugin{ std::move(prop["dds.sec.log.plugin"]) };
    if (!log_plugin.empty())
    {
      PParam.rtps.properties.properties().emplace_back("dds.sec.log.plugin", log_plugin);
      const std::string path = "dds.sec.log." + log_plugin;
      const std::vector<Properties*>& log_leaf = prop.getNode(path).getLeaf();
      for (auto& lprop : log_leaf)
      {
        std::string key(path + "." + lprop->getName()), value(lprop->getValue());
        PParam.rtps.properties.properties().emplace_back(key, value);
      }
    }
  }

  /*!
   * @if jp
   * @brief XMLファイルが設定済みかを判定
   *
   * @return true：設定済み、false：未設定
   *
   * @else
   * @brief
   *
   * @return
   *
   *
   * @endif
   */
  bool FastDDSManager::xmlConfigured()
  {
    return !m_xml_profile_file.empty();
  }
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#pragma warning(pop)
#endif
