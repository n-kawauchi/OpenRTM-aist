// -*- C++ -*-
/*!
 * @file Manager.h
 * @brief RTComponent manager class
 * @date $Date: 2007-04-13 18:02:01 $
 * @author Noriaki Ando <n-ando@aist.go.jp>
 *
 * Copyright (C) 2003-2005
 *     Task-intelligence Research Group,
 *     Intelligent Systems Research Institute,
 *     National Institute of
 *         Advanced Industrial Science and Technology (AIST), Japan
 *     All rights reserved.
 *
 * $Id: Manager.cpp,v 1.7 2007-04-13 18:02:01 n-ando Exp $
 *
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2007/01/14 19:42:30  n-ando
 * The activate() function now performs POA manager activation and
 * invoking ModuleInitProc.
 * Debugging messages are now output to system logger.
 *
 * Revision 1.5  2007/01/12 14:32:36  n-ando
 * Some headers that should be included in the distribution package were
 * added to the EXTRA_DIST entry.
 *
 * Revision 1.4  2007/01/09 15:11:24  n-ando
 * Now RTObject is activated itself. The Manager does nothing.
 *
 * Revision 1.3  2006/11/06 01:31:50  n-ando
 * Some Manager's functions has been implemented.
 * - Component creation process
 * - System logger initialization
 *
 * Revision 1.2  2006/10/25 17:27:57  n-ando
 * Component factory registration and relative functions are implemented.
 *
 * Revision 1.1  2006/10/17 10:21:17  n-ando
 * The first commitment.
 *
 *
 */


#include <rtm/Manager.h>
#include <rtm/ManagerConfig.h>
#include <rtm/Properties.h>
#include <rtm/ModuleManager.h>
#include <rtm/CorbaNaming.h>
#include <rtm/CorbaObjectManager.h>
#include <rtm/NamingManager.h>
#include <rtm/RTC.h>
#include <rtm/StringUtil.h>
#include <rtm/PeriodicExecutionContext.h>
#include <rtm/ExtTrigExecutionContext.h>
#include <rtm/RTCUtil.h>
#include <fstream>
#include <ace/Signal.h>
#include <rtm/TimeValue.h>
#include <rtm/Timer.h>


//static sig_atomic_t g_mgrActive = true;
extern "C" void handler (int)
{
  RTC::Manager::instance().terminate();
}

namespace RTC
{
  Manager* Manager::manager = NULL;
  ACE_Thread_Mutex Manager::mutex;
  

  /*!
   * @if jp
   * @brief Protected コンストラクタ
   * @else
   * @brief Protected Constructor
   * @endif
   */
  Manager::Manager()
    : m_initProc(NULL),
      m_Logbuf(), m_MedLogbuf(m_Logbuf), rtcout(m_MedLogbuf),
      m_runner(NULL), m_terminator(NULL)
      
  {
    new ACE_Sig_Action((ACE_SignalHandler) handler, SIGINT);
  }


  /*!
   * @if jp
   * @brief Protected コピーコンストラクタ
   * @else
   * @brief Protected Copy Constructor
   * @endif
   */
  Manager::Manager(const Manager& manager)
    : m_initProc(NULL),
      m_Logbuf(), m_MedLogbuf(m_Logbuf), rtcout(m_MedLogbuf),
      m_runner(NULL), m_terminator(NULL)
  {
    new ACE_Sig_Action((ACE_SignalHandler) handler, SIGINT);
  }


  /*!
   * @if jp
   * @brief マネージャの初期化
   * @else
   * @brief Initializa manager
   * @endif
   */
  Manager* Manager::init(int argc, char** argv)
  {
    // DCL for singleton
    if (!manager)
      {
	ACE_Guard<ACE_Thread_Mutex> guard(mutex);
	if (!manager)
	  {
	    manager = new Manager();
	    manager->initManager(argc, argv);
	    manager->initLogger();
	    manager->initORB();
	    manager->initNaming();
	    manager->initExecContext();
	    manager->initTimer();
	  }
      }
    return manager;
  }


  /*!
   * @if jp
   * @brief マネージャのインスタンスの取得
   * @else
   * @brief Get instance of the manager
   * @return The only instance reference of the manager
   * @endif
   */ 
  Manager& Manager::instance()
  {
    // DCL for singleton
    if (!manager)
      {
	ACE_Guard<ACE_Thread_Mutex> guard(mutex);
	if (!manager)
	  {
	    manager = new Manager();
	    manager->initManager(0, NULL);
	    manager->initLogger();
	    manager->initORB();
	    manager->initNaming();
	    manager->initExecContext();
	    manager->initTimer();
	  }
      }
    return *manager;
  }




  void Manager::terminate()
  {
    if (m_terminator != NULL)
      m_terminator->terminate();
  }

  void Manager::shutdown()
  {
    RTC_TRACE(("Manager::shutdown()"));
    shutdownNaming();
    shutdownORB();
    shutdownManager();
    // 終了待ち合わせ
    if (m_runner != NULL)
      {
	m_runner->wait();
      }
    else
      {
	join();
      }
    shutdownLogger();
  }

  void Manager::join()
  {
    RTC_TRACE(("Manager::wait()"));
    {
      ACE_Guard<ACE_Thread_Mutex> guard(m_terminate.mutex);
      ++m_terminate.waiting;
    }
    while (1)
      {
	{
	  ACE_Guard<ACE_Thread_Mutex> guard(m_terminate.mutex);
	  if (m_terminate.waiting > 1) break;
	}
	usleep(100000);
      }
  }


  void Manager::setModuleInitProc(ModuleInitProc proc)
  {
    m_initProc = proc;
  }


  bool Manager::activateManager()
  {
    RTC_TRACE(("Manager::activateManager()"));

    try
      {
	this->getPOAManager()->activate();

	if (m_initProc != NULL)
	  m_initProc(this);
      }
    catch (...)
      {
	return false;
      }
    return true;
  }


  void Manager::runManager(bool no_block)
  {
    if (no_block)
      {
	RTC_TRACE(("Manager::runManager(): non-blocking mode"));
	m_runner = new OrbRunner(m_pORB);
	m_runner->open(0);
      }
    else
      {
	RTC_TRACE(("Manager::runManager(): blocking mode"));
	m_pORB->run();
	RTC_TRACE(("Manager::runManager(): ORB was terminated"));
	join();
      }
    return;
  }




  //============================================================
  // Module management
  //============================================================
  /*!
   * @if jp
   * @brief モジュールのロード
   * @else
   * @brief Load module
   * @endif
   */  
  void Manager::load(const char* fname, const char* initfunc)
  {
    RTC_TRACE(("Manager::load()"));
    m_module->load(fname, initfunc);
    return;
  }


  /*!
   * @if jp
   * @brief モジュールのアンロード
   * @else
   * @brief Unload module
   * @param pathname Module file name
   * @endif
   */
  void Manager::unload(const char* fname)
  {
    RTC_TRACE(("Manager::unload()"));
    m_module->unload(fname);
    return;
  }


  /*!
   * @if jp
   * @brief 全モジュールのアンロード
   * @else
   * @brief Unload module
   * @endif
   */ 
  void Manager::unloadAll()
  {
    RTC_TRACE(("Manager::unloadAll()"));
    m_module->unloadAll();
    return;
  }

  
  /*!
   * @if jp
   * @brief ロード済みのモジュールリストを取得する
   * @else
   * @brief Get loaded module names
   * @endif
   */
  std::vector<std::string> Manager::getLoadedModules()
  {
    RTC_TRACE(("Manager::getLoadedModules()"));
    return m_module->getLoadedModules();
  }


  /*!
   * @if jp
   * @brief ロード可能なモジュールリストを取得する
   * @else
   * @brief Get loadable module names
   * @endif
   */
  std::vector<std::string> Manager::getLoadableModules()
  {    
    RTC_TRACE(("Manager::getLoadableModules()"));
    return m_module->getLoadableModules();
  }

  //============================================================
  // Component factory management
  //============================================================
  /*!
   * @if jp
   * @brief RTコンポーネントファクトリを登録する
   * @else
   * @brief Register RT-Component Factory
   * @endif
   */
  bool Manager::registerFactory(Properties& profile,
				RtcNewFunc new_func,
				RtcDeleteFunc delete_func)
  {
    RTC_TRACE(("Manager::registerFactory(%s)", profile["type_name"].c_str()));
    try
      {    
	FactoryBase* factory;
	factory = new FactoryCXX(profile, new_func, delete_func);
	m_factory.registerObject(factory);
	return true;
      }
    catch (...)
      {
	return false;
      }
  }
  
  bool Manager::registerECFactory(const char* name,
				  ECNewFunc new_func,
				  ECDeleteFunc delete_func)
  {
    RTC_TRACE(("Manager::registerECFactory(%s)", name));
    try
      {    
	ECFactoryBase* factory;
	factory = new ECFactoryCXX(name, new_func, delete_func);
	m_ecfactory.registerObject(factory);
	return true;
      }
    catch (...)
      {
	return false;
      }
    return false;
  }
  
  
  /*!
   * @if jp
   * @brief ファクトリ全リストを取得する
   * @else
   * @brief Get the list of all RT-Component Factory
   * @endif
   */
  std::vector<std::string> Manager::getModulesFactories()
  {
    RTC_TRACE(("Manager::getModulesFactories()"));

    ModuleFactories m;
    return m_factory.for_each(m).modlist;
  }
  

  //============================================================
  // Component management
  //============================================================
  /*!
   * @if jp
   * @brief RTコンポーネントを生成する
   * @else
   * @brief Create RT-Component
   * @endif
   */
  RtcBase* Manager::createComponent(const char* module_name)
  {
    RTC_TRACE(("Manager::createComponent(%s)", module_name));

    //------------------------------------------------------------
    // Create Component
    RtcBase* comp;
    comp = m_factory.find(module_name)->create(this);
    if (comp == NULL) return NULL;
    RTC_TRACE(("RTC Created: %s", module_name));

    //------------------------------------------------------------
    // Load configuration file specified in "rtc.conf"
    //
    // rtc.conf:
    //   [category].[type_name].config_file = file_name
    //   [category].[instance_name].config_file = file_name
    {
      std::string category(comp->getCategory());
      std::string type_name(comp->getTypeName());
      std::string inst_name(comp->getInstanceName());

      std::string type_conf(category + "." + type_name + ".config_file");
      std::string name_conf(category + "." + inst_name + ".config_file");


      Properties type_prop, name_prop;

      // Load "category.instance_name.config_file"
      if (!m_config[name_conf].empty())
	{
	  std::ifstream conff(m_config[name_conf].c_str());
	  if (!conff.fail())
	    {
	      name_prop.load(conff);
	    }
	}

      if (!m_config[type_conf].empty())
	{
	  std::ifstream conff(m_config[type_conf].c_str());
	  if (!conff.fail())
	    {
	      type_prop.load(conff);
	    }
	}
      // Merge Properties. type_prop is merged properties
      type_prop << name_prop;
      comp->getProperties() << type_prop;

    }


    //------------------------------------------------------------
    // Format component's name for NameService
    std::string naming_formats;
    Properties& comp_prop(comp->getProperties());

    naming_formats += m_config["naming_formats"];
    naming_formats += ", " + comp_prop["naming_formats"];
    naming_formats = flatten(unique_sv(split(naming_formats, ",")));

    std::string naming_names;
    naming_names = formatString(naming_formats.c_str(), comp->getProperties());
    comp->getProperties()["naming_names"] = naming_names;


    //------------------------------------------------------------
    // Component initialization
    bindExecutionContext(comp);
    comp->initialize();

    //------------------------------------------------------------
    // Bind component to naming service
    registerComponent(comp);
    return comp;
  }


  /*!
   * @if jp
   * @brief RTコンポーネントを直接 Manager に登録する
   * @else
   * @brief Register RT-Component directly without Factory
   * @endif
   */
  bool Manager::registerComponent(RtcBase* comp)
  {
    RTC_TRACE(("Manager::registerComponent(%s)", comp->getInstanceName()));
    m_compManager.registerObject(comp);

    std::vector<std::string> names(comp->getNamingNames());

    for (int i(0), len(names.size()); i < len; ++i)
      {
	RTC_TRACE(("Bind name: %s", names[i].c_str()));
	m_namingManager->bindObject(names[i].c_str(), comp);
      }
    return true;
  }


  bool Manager::bindExecutionContext(RtcBase* comp)
  {
    RTC_TRACE(("Manager::bindExecutionContext()"));
    RTC_TRACE(("ExecutionContext type: %s", 
	       m_config.getProperty("exec_cxt.periodic.type").c_str()));

    RTObject_var rtobj;
    rtobj = comp->getObjRef();

    ExecutionContextBase* exec_cxt;

    if (isDataFlowParticipant(rtobj))
      {
	const char* ectype
	  = m_config.getProperty("exec_cxt.periodic.type").c_str();
	exec_cxt = m_ecfactory.find(ectype)->create();
	const char* rate
	  = m_config.getProperty("exec_cxt.periodic.rate").c_str();
	exec_cxt->set_rate(atof(rate));
      }
    /*
    else if (isDataFlowComposite(rtobj))
      {
      }
    else if (isFsmParticipant(rtobj))
      {
      }      
    else if (isFsm(rtobj))
      {
      }      
    else if (isMultiModeObject(rtobj))
      {
      }
    */
    else
      {
	const char* ectype
	  = m_config.getProperty("exec_cxt.evdriven.type").c_str();
	exec_cxt = m_ecfactory.find(ectype)->create();
      }
    exec_cxt->add(rtobj);
    return true;
  }


  /*!
   * @if jp
   * @brief Manager に登録されているRTコンポーネントを削除する
   * @else
   * @brief Unregister RT-Component that is registered in the Manager
   * @endif
   */
  void Manager::deleteComponent(const char* instance_name)
  {
    RTC_TRACE(("Manager::deleteComponent(%s)", instance_name));
    //    RtcBase* comp;
    //comp = m_component.find(instance_name);

    //    m_naming->unregister(comp);
    //    m_activator->deactivate(comp);

    //    comp->finalize();
    //    m_component->unregisterObject(instance_name);
    //    delete comp;
  }


  /*!
   * @if jp
   * @brief Manager に登録されているRTコンポーネントを取得する
   * @else
   * @brief Get RT-Component's pointer
   * @endif
   */
  RtcBase* Manager::getComponent(const char* instance_name)
  {
    RTC_TRACE(("Manager::getComponent(%s)", instance_name));
    //    return m_component->find(instance_name);
    return NULL;
  }


  /*!
   * @if jp
   * @brief Manager に登録されている全RTコンポーネントを取得する
   * @else
   * @brief Get all RT-Component's pointer
   * @endif
   */
  std::vector<RtcBase*> Manager::getComponents()
  {
    RTC_TRACE(("Manager::getComponents()"));
    return m_compManager.getObjects();
  }


  //============================================================
  // CORBA 関連
  //============================================================
  /*!
   * @if jp
   * @brief ORB のポインタを取得する
   * @else
   * @brief Get the pointer to the ORB
   * @endif
   */
  CORBA::ORB_ptr Manager::getORB()
  {
    RTC_TRACE(("Manager::getORB()"));
    return m_pORB;
  }


  /*!
   * @if jp
   * @brief Manager が持つ RootPOA のポインタを取得する
   * @else
   * @brief Get the pointer to the RootPOA 
   * @endif
   */
  PortableServer::POA_ptr Manager::getPOA()
  {
    RTC_TRACE(("Manager::getPOA()"));
    return m_pPOA;
  }

  PortableServer::POAManager_ptr Manager::getPOAManager()
  {
    RTC_TRACE(("Manager::getPOAManager()"));
    return m_pPOAManager;
  }

  


  //============================================================
  // Protected functions
  //============================================================


  //============================================================
  // Manager initialization and finalization
  //============================================================
  /*!
   * @if jp
   * @brief Manager の内部初期化処理
   * @else
   * @brief Manager internal initialization
   * @endif
   */
  void Manager::initManager(int argc, char** argv)
  {
    ManagerConfig config(argc, argv);
    config.configure(m_config);
    m_config["logger.file_name"] = 
      formatString(m_config["logger.file_name"].c_str(), m_config);
    
    m_module = new ModuleManager(m_config);
    m_terminator = new Terminator(this);
    
    if (toBool(m_config["timer.enable"], "YES", "NO", true))
      {
	TimeValue tm(0, 100000);
	std::string tick(m_config["timer.tick"]);
	if (!tick.empty())
	  {
	    tm = atof(tick.c_str());
	    m_timer = new ::Timer(tm);
	    m_timer->start();
	  }
      }
  }


  /*!
   * @if jp
   * @brief Manager の終了処理
   * @else
   * @brief Manager internal finalization
   * @endif
   */
  void Manager::shutdownManager()
  {
    RTC_TRACE(("Manager::shutdownManager()"));
  }



  //============================================================
  // Logger initialization and finalization
  //============================================================
  /*!
   * @if jp
   * @brief System logger の初期化
   * @else
   * @brief System logger initialization
   * @endif
   */
  bool Manager::initLogger()
  {
    rtcout.setLogLevel("SILENT");
    
    if (toBool(m_config["logger.enable"], "YES", "NO", true))
      {
	std::string logfile(m_config["logger.file_name"]);
	if (logfile == "") logfile = "./rtc.log";
	
	// Open logfile
	m_Logbuf.open(logfile.c_str(), std::ios::out | ios::app);

	if (!m_Logbuf.is_open())
	  {
	    std::cerr << "Error: cannot open logfile: "
		      << logfile << std::endl;
	    return false;
	  }

	// Set suffix for log entry haeader.
	m_MedLogbuf.setSuffix(m_config["manager.name"]);
	
	// Set date format for log entry header
	m_MedLogbuf.setDateFmt(m_config["logger.date_format"]);

	// Loglevel was set from configuration file.
	rtcout.setLogLevel(m_config["logger.log_level"]);
	
	// Log stream mutex locking mode
	rtcout.setLogLock(toBool(m_config["logger.stream_lock"],
				 "enable", "disable", false));
	
	RTC_INFO(("%s", m_config["openrtm.version"].c_str()));
	RTC_INFO(("Copyright (C) 2003-2007"));
	RTC_INFO(("  Noriaki Ando"));
	RTC_INFO(("  Task-intelligence Research Group,"));
	RTC_INFO(("  Intelligent Systems Research Institute, AIST"));
	RTC_INFO(("Manager starting."));
	RTC_INFO(("Starting local logging."));
      }
	
    return true;;
  }


  /*!
   * @if jp
   * @brief System Logger の終了処理
   * @else
   * @brief System Logger finalization
   * @endif
   */
  void Manager::shutdownLogger()
  {
    RTC_TRACE(("Manager::shutdownLogger()"));
    rtcout.flush();
    if (m_Logbuf.is_open())
      m_Logbuf.close();
  }

  
  //============================================================
  // ORB initialization and finalization
  //============================================================
  /*!
   * @if jp
   * @brief CORBA ORB の初期化処理
   * @else
   * @brief CORBA ORB initialization
   * @endif
   */
  bool Manager::initORB()
  {
    RTC_TRACE(("Manager::initORB()"));
    // Initialize ORB
    try
      {
	std::vector<std::string> args(split(createORBOptions(), " "));
	char** argv(toArgv(args));
	int argc(args.size());

	// ORB initialization
	m_pORB = CORBA::ORB_init(argc, argv);

	// Get the RootPOA
	CORBA::Object_var obj = m_pORB->resolve_initial_references("RootPOA");
	m_pPOA = PortableServer::POA::_narrow(obj);
	if (CORBA::is_nil(m_pPOA))
	  {
	    RTC_ERROR(("Could not resolve RootPOA."));
	    return false;
	  }
	// Get the POAManager
	m_pPOAManager = m_pPOA->the_POAManager();
	m_objManager = new CorbaObjectManager(m_pORB, m_pPOA);
      }
    catch (...)
      {
	RTC_ERROR(("Exception: Caught unknown exception in initORB()." ));
	return false;
      }
    return true;
  }

  
  /*!
   * @if jp
   * @brief ORB のコマンドラインオプション作成
   * @else
   * @brief ORB command option creation
   * @endif
   */
  std::string Manager::createORBOptions()
  {
    std::string opt(m_config["corba.args"]);
    std::string corba(m_config["corba.id"]);
    std::string endpoint(m_config["corba.endpoint"]);

    if (!endpoint.empty())
      {
	if (!opt.empty()) opt += " ";
	if (corba == "omniORB")   opt = "-ORBendPoint giop:tcp:" + endpoint;
	else if (corba == "TAO")  opt = "-ORBEndPoint iiop://" + endpoint;
	else if (corba == "MICO") opt = "-ORBIIOPAddr inet:" + endpoint;
      }
    return opt;
  }


  /*!
   * @if jp
   * @brief ORB の終了処理
   * @else
   * @brief ORB finalization
   * @endif
   */
  void Manager::shutdownORB()
  {
    RTC_TRACE(("Manager::shutdownORB()"));
    while (m_pORB->work_pending())
      {
	if (m_pORB->work_pending())
	  m_pORB->perform_work();
      }
    RTC_DEBUG(("No pending works of ORB. Shutting down POA and ORB."));

    if (!CORBA::is_nil(m_pPOA))
      {
	try
	  {
	    m_pPOAManager->deactivate(true, true);
	    RTC_DEBUG(("POA Manager was deactivated."));
	    m_pPOA->destroy(true, true);
	    m_pPOA = PortableServer::POA::_nil();
	    RTC_DEBUG(("POA was destroid."));
	  }
	catch (CORBA::SystemException& ex)
	  {
	    RTC_ERROR(("Caught SystemException during root POA destruction"));
	  }
	catch (...)
	  {
	    RTC_ERROR(("Caught unknown exception during POA destruction."));
	  }
      }

   if (!CORBA::is_nil(m_pORB))
     {
       try
	 {
	   m_pORB->shutdown(true);
	   RTC_DEBUG(("ORB was shutdown."));
	   m_pORB->destroy();
	   RTC_DEBUG(("ORB was destroied."));
	   m_pORB = CORBA::ORB::_nil();
	 }
       catch (CORBA::SystemException& ex)
	 {
	   RTC_ERROR(("Caught CORBA::SystemException during ORB shutdown"));
	 }
       catch (...)
	 {
	   RTC_ERROR(("Caught unknown exception during ORB shutdown."));
	 }
     }

  }


  //============================================================
  // Naming initialization and finalization
  //============================================================
  /*!
   * @if jp
   * @brief NamingManager の初期化
   * @else
   * @brief NamingManager initialization
   * @endif
   */
  bool Manager::initNaming()
  {
    RTC_TRACE(("Manager::initNaming()"));

    m_namingManager = new NamingManager(this);

    // If NameService is disabled, return immediately
    if (!toBool(m_config["name_svc.enable"], "YES", "NO", true)) return true;

    // NameServer registration for each method and servers
    std::vector<std::string> meth(split(m_config["name_svc.type"], ","));

    for (int i(0), len_i(meth.size()); i < len_i; ++i)
      {
	std::vector<std::string> names;
	names = split(m_config[meth[i] + ".nameservers"], ",");


	for (int j(0), len_j(names.size()); j < len_j; ++j)
	  {
	    RTC_TRACE(("Register Naming Server: %s/%s", \
		       meth[i].c_str(), names[j].c_str()));	
	    m_namingManager->registerNameServer(meth[i].c_str(),
						names[j].c_str());
	  }
      }

    // NamingManager Timer update initialization
    if (toBool(m_config["name_svc.update.enable"], "YES", "NO", true))
      {
	TimeValue tm(10, 0); // default interval = 10sec for safty
	std::string intr(m_config["name_svc.update.interval"]);
	if (!intr.empty())
	  {
	    tm = atof(intr.c_str());
	  }
	if (m_timer != NULL)
	  {
	    m_timer->registerListenerObj(m_namingManager, 
					 &NamingManager::update, tm);
	  }
      }
    return true;
  }


  /*!
   * @if jp
   * @brief NamingManager の終了処理
   * @else
   * @brief NamingManager finalization
   * @endif
   */
  void Manager::shutdownNaming()
  {
    RTC_TRACE(("Manager::shutdownNaming()"));
    m_namingManager->unbindAll();
  }


  //============================================================
  // Naming initialization and finalization
  //============================================================
  /*!
   * @if jp
   * @brief ExecutionContextManager の初期化
   * @else
   * @brief ExecutionContextManager initialization
   * @endif
   */
  bool Manager::initExecContext()
  {
    RTC_TRACE(("Manager::initExecContext()"));
    PeriodicExecutionContextInit(this);
    ExtTrigExecutionContextInit(this);
    return true;
  }

  bool Manager::initTimer()
  {
    return true;
  }

  void Manager::shutdownComponents()
  {
    RTC_TRACE(("Manager::shutdownComponents()"));
  }



  bool Manager::mergeProperty(Properties& prop, const char* file_name)
  {
    if (file_name[0] != '\0')
      {
	std::ifstream conff(file_name);
	if (!conff.fail())
	  {
	    prop.load(conff);
	    conff.close();
	    return true;
	  }
      }
    return false;
  }

  std::string Manager::formatString(const char* naming_format,
				    Properties& prop)
  {
    std::string name(naming_format), str("");
    std::string::iterator it, it_end;
    int count(0);

    it = name.begin();
    it_end = name.end();
    for ( ; it != it_end; ++it)
      {
	char c(*it);
	if (c == '%')
	  {
	    ++count;
	    if (!(count % 2)) str.push_back((*it));
	  }
	else
	  {
	    if (count > 0 && (count % 2))
	      {
		count = 0;
		if      (c == 'n')  str += prop["instance_name"];
		else if (c == 't')  str += prop["type_name"];
		else if (c == 'm')  str += prop["type_name"];
		else if (c == 'v')  str += prop["version"];
		else if (c == 'V')  str += prop["vendor"];
		else if (c == 'c')  str += prop["category"];
		else if (c == 'h')  str += m_config["os.hostname"];
		else if (c == 'M')  str += m_config["manager.name"];
		else if (c == 'p')  str += m_config["manager.pid"];
		else str.push_back(c);
	      }
	    else
	      {
		count = 0;
		str.push_back(c);
	      }
	  }
      }
    return str;
  }

};
