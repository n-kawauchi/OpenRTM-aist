// -*- C++ -*-
/*!
 * @file FactoryInit.cpp
 * @brief factory initialization function
 * @date $Date: 2008-03-06 06:58:40 $
 * @author Noriaki Ando <n-ando@aist.go.jp>
 *
 * Copyright (C) 2009-2017
 *     Task-intelligence Research Group,
 *     Intelligent Systems Research Institute,
 *     National Institute of
 *         Advanced Industrial Science and Technology (AIST), Japan
 *     All rights reserved.
 *
 * $Id: Manager.cpp 1296 2009-04-27 08:43:24Z n-ando $
 *
 */

// Logstream
#include <rtm/LogstreamFile.h>

// Buffers
#include <rtm/CdrRingBuffer.h>

// Threads
#include <rtm/DefaultPeriodicTask.h>

// default Publishers
#include <rtm/PublisherFlush.h>
#include <rtm/PublisherNew.h>
#include <rtm/PublisherPeriodic.h>

// default Provider/Consumer
#include <rtm/InPortCorbaCdrProvider.h>
#include <rtm/InPortCorbaCdrConsumer.h>
#include <rtm/OutPortCorbaCdrConsumer.h>
#include <rtm/OutPortCorbaCdrProvider.h>
#include <rtm/InPortDirectProvider.h>
#include <rtm/InPortDirectConsumer.h>
#include <rtm/InPortSHMProvider.h>
#include <rtm/InPortSHMConsumer.h>
#include <rtm/OutPortSHMProvider.h>
#include <rtm/OutPortSHMConsumer.h>

// RTC name numbering policy
#include <rtm/NumberingPolicy.h>

void FactoryInit()
{
    // Logstream
    LogstreamFileInit();

    // Buffers
    CdrRingBufferInit();

    // Threads
    DefaultPeriodicTaskInit();

    // Publishers
    PublisherFlushInit();
    PublisherNewInit();
    PublisherPeriodicInit();

    // Providers/Consumer
    InPortCorbaCdrProviderInit();
    InPortCorbaCdrConsumerInit();
    OutPortCorbaCdrConsumerInit();
    OutPortCorbaCdrProviderInit();
    InPortDirectProviderInit();
    InPortDirectConsumerInit();
    InPortSHMProviderInit();
    InPortSHMConsumerInit();
    OutPortSHMProviderInit();
    OutPortSHMConsumerInit();

    // Naming Policy
    ProcessUniquePolicyInit();
    //    NodeUniquePolicyInit();
    //    NamingServiceUniquePolicyInit();
}
