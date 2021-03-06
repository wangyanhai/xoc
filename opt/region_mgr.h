/*@
XOC Release License

Copyright (c) 2013-2014, Alibaba Group, All rights reserved.

    compiler@aliexpress.com

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Su Zhenyu nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

author: Su Zhenyu
@*/
#ifndef __REGION_MGR_H__
#define __REGION_MGR_H__

namespace xoc {

typedef enum {
    RU_UNDEF = 0,

    //Region is black box with
    //Black box is single entry, single exit.
    RU_BLX,

    //Sub region is the region which contains a list of IRs,
    //Sub region must be single entry, multiple exits.
    RU_SUB,

    //Region is exception region.
    //Exception is single entry, multiple exits.
    RU_EH,

    //Region is function unit
    //Function unit is single entry, multiple exits.
    RU_FUNC,

    //Region is whole program spectrum.
    //Program region is single entry, multiple exits.
    RU_PROGRAM,
} REGION_TYPE;

class Region;
class CallGraph;
class IPA;

//
//START RegionMgr
//
//Region Manager is the top level manager.
#define RM_label_count(r)        ((r)->m_label_count)
class RegionMgr {
friend class Region;
protected:
    #ifdef _DEBUG_
    UINT m_num_allocated;
    #endif
    Vector<Region*> m_id2ru;
    BitSetMgr m_bs_mgr;
    DefMiscBitSetMgr m_sbs_mgr;
    SymTab m_sym_tab;
    TypeMgr m_type_mgr;
    VarMgr * m_var_mgr;
    MD const* m_str_md;
    MDSystem * m_md_sys;
    CallGraph * m_call_graph;
    UINT m_ru_count;
    List<UINT> m_free_ru_id;
    UINT m_label_count;
    bool m_is_regard_str_as_same_md;
    TargInfo * m_targinfo;

protected:
    void estimateEV(OUT UINT & num_call,
                    OUT UINT & num_ru,
                    bool scan_call,
                    bool scan_inner_region);

public:
    explicit RegionMgr() : m_sym_tab(0)
    {
        ASSERT0(verifyPreDefinedInfo());
        #ifdef _DEBUG_
        m_num_allocated = 0;
        #endif
        m_ru_count = 1;
        m_label_count = 1;
        m_var_mgr = NULL;
        m_md_sys = NULL;
        m_is_regard_str_as_same_md = true;
        m_str_md = NULL;
        m_call_graph = NULL;
        m_targinfo = NULL;
        m_sym_tab.init(64);
    }
    COPY_CONSTRUCTOR(RegionMgr);
    virtual ~RegionMgr();

    SYM * addToSymbolTab(CHAR const* s) { return m_sym_tab.add(s); }

    //This function will establish a map between region and its id.
    void addToRegionTab(Region * ru);

    //Allocate Region.
    virtual Region * allocRegion(REGION_TYPE rt);

    //Allocate VarMgr.
    virtual VarMgr * allocVarMgr();

    //Allocate CallGraph.
    virtual CallGraph * allocCallGraph(UINT edgenum, UINT vexnum);

    //Allocate IPA module.
    IPA * allocIPA(Region * program);

    //Destroy specific region by given id.
    void deleteRegion(Region * ru, bool collect_id = true);

    void dumpRelationGraph(CHAR const* name);

    //Dump regions recorded via addToRegionTab().
    void dump(bool dump_inner_region);

    BitSetMgr * get_bs_mgr() { return &m_bs_mgr; }
    DefMiscBitSetMgr * get_sbs_mgr() { return &m_sbs_mgr; }
    virtual Region * get_region(UINT id) { return m_id2ru.get(id); }
    UINT getNumOfRegion() const { return (UINT)(m_id2ru.get_last_idx() + 1); }
    VarMgr * get_var_mgr() { return m_var_mgr; }
    MD const* genDedicateStrMD();
    MDSystem * get_md_sys() { return m_md_sys; }
    SymTab * get_sym_tab() { return &m_sym_tab; }
    TypeMgr * get_type_mgr() { return &m_type_mgr; }
    CallGraph * get_call_graph() const { return m_call_graph; }
    VarMgr * get_var_mgr() const { return m_var_mgr; }
    TargInfo * get_targ_info() const { return m_targinfo; }

    //Register exact MD for each global variable.
    //Note you should call this function as early as possible, e.g, before process
    //all regions. Because that will assign smaller MD id to global variable.
    void registerGlobalMD();

    //Initialize VarMgr structure and MD system.
    //It is the first thing you should do after you declared a RegionMgr.
    inline void initVarMgr()
    {
        ASSERT(m_var_mgr == NULL, ("VarMgr already initialized"));
        m_var_mgr = allocVarMgr();
        ASSERT0(m_var_mgr);

        ASSERT(m_md_sys == NULL, ("MDSystem already initialized"));
        m_md_sys = new MDSystem(m_var_mgr);
        ASSERT0(m_md_sys);
    }

    //Scan call site and build call graph.
    void buildCallGraph(OptCtx & oc, bool scan_call, bool scan_inner_region);

    Region * newRegion(REGION_TYPE rt);

    void set_targ_info(TargInfo * ti) { m_targinfo = ti; }

    //Process region in the form of function type.
    virtual bool processFuncRegion(IN Region * func, OptCtx * oc);

    //Process top-level region unit.
    //Top level region unit should be program unit.
    virtual bool processProgramRegion(IN Region * program, OptCtx * oc);

    bool verifyPreDefinedInfo();
};
//END RegionMgr

} //namespace xoc
#endif
