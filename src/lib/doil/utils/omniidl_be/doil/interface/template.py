#!/usr/bin/env python3
# -*- coding: shift_jis -*-
# -*- python -*-
#
#  @file template.py
#  @brief doil interface header template
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

types_h = """\
// -*- C++ -*-
/*!
 * @file [types_h] 
 * @brief C++ interface definition for doil
 * @date $Date$
 * @author This file was automatically generated by omniidl/doil backend
 *         This file was generated from [idl_fname].
 *
 * $Id$
 */

#ifndef [types_include_guard] 
#define [types_include_guard] 

#include <vector>
#include <string>

[for inc in include_h]
#include <[inc]>
[endfor]

[for iinc in idl_includes]
#include <[iinc.types_h_path]>
[endfor]

[for decl in declarations]
[decl]
[endfor]
#endif // [types_include_guard]

"""

ns_decl = """
[for-inv ens in end_ns]
}; // namespace [ens] 
[endfor]
[for bns in begin_ns]
namespace [bns] 
{
[endfor]

"""

ifacefwd_decl = """\
  // forward declaration of interface
  class [local.iface_name];

"""

struct_decl = """\
  // struct
  struct [local.name] 
  {
[for mem in members]
    [mem.local.member_type] [mem.local.member_name];
[endfor]
  };

"""

structfwd_decl = """\
  // struct
  struct [local.name];

"""

union_decl = """\
  // union
  class [local.name] 
  {
  public:
    [local.name](){}
    virtual ~[local.name](){}

[for cas in cases]
    [local.name]([cas.local.case_type] var)
      : m_type([cas.local.discriminator]),
        m_[cas.local.case_member](var){}

    [cas.local.case_type] get_[cas.local.case_member]() const
    {
      return m_[cas.local.case_member];
    }
    void set_[cas.local.case_member]([cas.local.case_type] var)
    {
      m_type = [cas.local.discriminator];
      m_[cas.local.case_member] = var;
    }
[endfor]

    [local.switch_fq_type] get_type() const
    {
      return m_type;
    }
  private:
    [local.switch_fq_type] m_type;
[for cas in cases]
    [cas.local.case_type] m_[cas.local.case_member];
[endfor]

  };

"""

unionfwd_decl = """\
  class [local.name];

"""

enum_decl = """\
  // enum
  enum [local.name] 
  {
[for mem in local.members]
[if-index mem is last]
    [mem]
[else]
    [mem],
[endif]
[endfor]

  };

"""

exception_decl = """\
  // struct
  struct [local.name] 
  {
[for mem in members]
    [mem.local.member_type] [mem.local.member_name];
[endfor]
  };

"""

typedef_decl = """\
  // typedef
  typedef [local.base_type] [local.derived_type];

"""



interface_h = """\
// -*- C++ -*-
/*!
 * @file [local.iface_h] 
 * @brief [local.iface_name] C++ interface definition for doil
 * @date $Date$
 * @author This file was automatically generated by omniidl/doil backend
 *
 * $Id$
 */

#ifndef [local.iface_include_guard] 
#define [local.iface_include_guard] 

#include <doil/ImplBase.h>
[for inc in local.include_h]
#include <[inc]>
[endfor]

[for iinc in idl_includes]
#include <[iinc.types_h_path]>
[endfor]
#include <[types_h_path]>

[for ih in inherits]
#include <[ih.local.iface_h_path]>
[endfor]

[for ns in local.iface_ns]
namespace [ns] 
{
[endfor]
  /*!
   * @if jp
   * @class [local.iface_name] 
   * @brief [local.iface_name] インターフェースクラス
   * @else
   * @class [local.iface_name] 
   * @brief [local.iface_name] interface class
   * @endif
   */
  class [local.iface_name] 
   : public virtual doil::LocalBase
[for ih in inherits]
[if-index ih is last],
     public virtual [ih.local.iface_name_fq]
[else],
     public virtual [ih.local.iface_name_fq]
[endif]
[endfor]

  {
  public:
    virtual ~[local.iface_name](){};

[for op in operations]
    virtual [op.return.local.retn_type] [op.name]([for a in op.args]
[if-index a is last][a.local.arg_type] [a.local.arg_name]
[else][a.local.arg_type][a.local.arg_name], [endif]
[endfor])
      throw ([for raise in op.raises]
[if-index raise is first]
[raise.local.name_fq]
[else]
,
             [raise.local.name_fq]
[endif]
[endfor]
) = 0;

[endfor]
  };
[for-inv ns in local.iface_ns]
}; // namespace [ns] 
[endfor]

#endif // [local.iface_include_guard]

"""


























a = """
[for td in typedefs]
[if td.corba.tk is "tk_sequence"][if td.corba.sq_tk is "tk_struct"]
[if td.local.deref_sq_type is st.local.base_type]
  typedef [td.local.deref_fq_type] [td.local.derived_type];
[endif][endif][endif]
[endfor]

  //------------------------------------------------------------
  // rest of typedef
  //------------------------------------------------------------
[for td in typedefs]
[if td.corba.tk is "tk_sequence"]
[if td.corba.sq_tk is "tk_struct"]
[elif td.corba.sq_tk is "tk_objref"]
[else]
  typedef [td.local.deref_fq_type] [td.local.derived_type];
[endif]
[else]
  typedef [td.local.deref_fq_type] [td.local.derived_type];
[endif]
[endfor]


[for ns in iface_ns]
}; // namespace [ns] 
[endfor]

#endif // [types_include_guard] 
"""
