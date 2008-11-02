#!/usr/bin/env python
# -*- python -*-
#
#  @file omniidl_be/doil/__init__.py
#  @brief doil backend for omniidl
#  @date $Date$
#  @author Noriaki Ando <n-ando@aist.go.jp>
# 
#  Copyright (C) 2008
#      Task-intelligence Research Group,
#      Intelligent Systems Research Institute,
#      National Institute of
#          Advanced Industrial Science and Technology (AIST), Japan
#      All rights reserved.
# 
#  $Id$
# 

#   Entrypoint to the doil backend

# modules from omniidl
from omniidl import idlast, idlvisitor, idlutil

# module from omniidl_be.cxx
from omniidl_be.cxx import support, ast

# module from omniidl_be.doil
from omniidl_be.doil import util, config

import re, sys, os.path

cpp_args = ["-D__OMNIIDL_CXX__"]
usage_string = """\
  -Wbm=<mapfile>  Specify type mapping rule file
  -Wbservant      Generate doil servant class for CORBA
  -Wbadapter      Generate doil adapter class for CORBA
  -Wbslice        Generate Ice slice file from CORBA IDL
  -Wbiceservant   Generate doil servant class for Ice
  -Wbiceadapter   Generate doil adapter class for Ice

[ options for servant class ]
  -Wbss=<suffix>  Specify suffix for generated servant files [default Servant]
  -Wbsp=<prefix>  Specify prefix for generated servant files [default None]
  -Wbsns=<ns>     Specify namespsace of generated servant class [default None]
  -Wbsdir=<ns>    Specify directory of generated servant header [default None]

[ options for adapter class ]
  -Wbas=<suffix>  Specify suffix for generated adapter class [default Adapter]
  -Wbap=<prefix>  Specify prefix for generated adapter class [default None]
  -Wbans=<ns>     Specify namespsace of generated adapter class [default None]
  -Wbadir=<ns>    Specify directory of generated adapter header [default None]

[ options for interface class ]
  -Wbis=<suffix>  Specify suffix for local interface class [default None]
  -Wbip=<prefix>  Specify prefix for local interface class [default I]
  -Wbins=<ns>     Specify namespsace of local interface class [default None]
  -Wbidir=<ns>    Specify directory of local interface class [default None]

"""

# Encountering an unknown AST node will cause an AttributeError exception
# to be thrown in one of the visitors. Store a list of those not-supported
# so we can produce a friendly error message later.
AST_unsupported_nodes = [ "Native", "StateMember", "Factory", "ValueForward",
                          "ValueBox", "ValueAbs", "Value" ]

def process_args(args):
    for arg in args:
        if arg == "d":
            config.state['Debug']                  = True
        elif arg[:2] == "m=":
            config.state['MappingFile']            = arg[2:]
        # generator options
        elif arg == "servant":
            config.state['Servant']                = True
        elif arg == "adapter":
            config.state['Adapter']                = True
        elif arg == "slice":
            config.state['IceSlice']               = True
        elif arg == "iceservant":
            config.state['IceServant']             = True
        elif arg == "iceadapter":
            config.state['IceAdapter']             = True
        # for servant
        elif arg[:3] == "ss=":
            config.state['ServantSuffix']          = arg[3:]
        elif arg[:3] == "sp=":
            config.state['ServantPrefix']          = arg[3:]
        elif arg[:4] == "sns=":
            config.state['ServantNs']              = arg[4:].split('::')
        elif arg[:5] == "sdir=":
            config.state['ServantDir']             = arg[5:]
        # for adapter
        elif arg[:3] == "as=":
            config.state['AdapterSuffix']          = arg[3:]
        elif arg[:3] == "ap=":
            config.state['AdapterPrefix']          = arg[3:]
        elif arg[:4] == "ans=":
            config.state['AdapterNs']              = arg[4:].split('::')
        elif arg[:5] == "adir=":
            config.state['AdapterDir']             = arg[5:]
        # for interface
        elif arg[:3] == "is=":
            config.state['IfaceSuffix']            = arg[3:]
        elif arg[:3] == "ip=":
            config.state['IfacePrefix']            = arg[3:]
        elif arg[:4] == "ins=":
            config.state['IfaceNs']                = arg[4:].split('::')
        elif arg[:5] == "idir=":
            config.state['IfaceDir']               = arg[5:]
        else:
            util.fatalError("Argument \"" + str(arg) + "\" is unknown")

run_before = 0


def run(tree, args):
    print tree
    print args

    """Entrypoint to the doil backend"""

    global run_before

    if run_before:
        util.fatalError("Sorry, the doil backend cannot process more "
                        "than one IDL file at a time.")
    run_before = 1

    dirname, filename = os.path.split(tree.file())
    basename,ext      = os.path.splitext(filename)
    config.state['Basename']  = basename
    config.state['Directory'] = dirname
    
    process_args(args)

    try:
        # Check the input tree only contains stuff we understand
        support.checkIDL(tree)

        # initialise the handy ast module
        ast.__init__(tree)

        # Initialise the descriptor generating code
        # currently doil does not use descriptor
        #        descriptor.__init__(tree)

        # CORBA Servant codes
        if config.state['Servant']:
            from omniidl_be.doil import servant
            servant.run(tree, config.state)

        # CORBA Adapter codes
        if config.state['Adapter']:
            from omniidl_be.doil import adapter
            adapter.run(tree)

        # Ice slice
        if config.state['IceSlice']:
            from omniidl_be.doil import iceslice
            iceslice.run(tree)

        # Ice Servant codes
        if config.state['IceServant']:
            from omniidl_be.doil import iceservant
            iceservant.run(tree)

        # Ice Adapter codes
        if config.state['IceAdapter']:
            from omniidl_be.doil import iceadapter
            iceadapter.run(tree)

    except AttributeError, e:
        name = e[0]
        unsupported_visitors = map(lambda x:"visit" + x,
                                   AST_unsupported_nodes[:])
        if name in unsupported_visitors:
            util.unsupportedIDL()
            
        util.fatalError("An AttributeError exception was caught")
    except SystemExit, e:
        # fatalError function throws SystemExit exception
        # delete all possibly partial output files
        for file in output.listAllCreatedFiles():
            os.unlink(file)
        
        raise e
    except:
        util.fatalError("An internal exception was caught")