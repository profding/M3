/*
 * Copyright (C) 2013, Nils Asmussen <nils@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of M3 (Microkernel for Minimalist Manycores).
 *
 * M3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * M3 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#include <m3/util/Sync.h>
#include <m3/Log.h>

#include "../../PEManager.h"
#include "../../SyscallHandler.h"
#include "../../KVPE.h"

using namespace m3;

void KVPE::start(int, char **, int) {
    // when exiting, the program will release one reference
    ref();
    activate_sysc_chan();
    _state = RUNNING;
    LOG(VPES, "Started VPE '" << _name << "' [id=" << _id << "]");
}

void KVPE::activate_sysc_chan() {
    alignas(DTU_PKG_SIZE) CoreConf conf;
    memset(&conf, 0, sizeof(conf));
    conf.coreid = core();
    conf.chans[ChanMng::SYSC_CHAN].dstcore = KERNEL_CORE;
    conf.chans[ChanMng::SYSC_CHAN].dstchan = ChanMng::SYSC_CHAN;
    conf.chans[ChanMng::SYSC_CHAN].label = reinterpret_cast<label_t>(&syscall_gate());
    DTU::get().set_target(SLOT_NO, core(), CONF_GLOBAL);
    Sync::memory_barrier();
    DTU::get().fire(SLOT_NO, DTU::WRITE, &conf, sizeof(conf));
}

Errors::Code KVPE::xchg_chan(size_t cid, MsgCapability *, MsgCapability *newcapobj) {
    // TODO later we need to use cmpxchg here
    alignas(DTU_PKG_SIZE) ChanConf conf;
    conf.dstcore = newcapobj ? newcapobj->obj->core : 0;
    conf.dstchan = newcapobj ? newcapobj->obj->chanid : 0;
    conf.label = newcapobj ? newcapobj->obj->label : label_t();
    conf.credits = newcapobj ? newcapobj->obj->credits : 0;
    DTU::get().set_target(SLOT_NO, core(), CONF_GLOBAL + offsetof(CoreConf, chans) + cid * sizeof(ChanConf));
    Sync::memory_barrier();
    DTU::get().fire(SLOT_NO, DTU::WRITE, &conf, sizeof(conf));
    return Errors::NO_ERROR;
}

KVPE::~KVPE() {
    LOG(VPES, "Deleting VPE '" << _name << "' [id=" << _id << "]");
    SyscallHandler::get().remove_session(this);
    detach_rbufs();
    free_deps();
}