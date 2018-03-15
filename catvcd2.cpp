/*
    Copyright (C) 2017 Nicolas Sauzede <nsauzede@laposte.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "vcdparser.h"

class MyVCDParser : public VCDParser {
private:
	uint64_t m_time;
	int m_donecount;
	int m_last_clrop;
	int m_last_latchabus;
	std::map<std::string, std::string> m_syms;
	std::unordered_map<std::string, std::string> m_names;
	std::unordered_map<std::string, std::string> m_values;
	std::vector<std::string> m_vsyms;
	std::vector<std::string> m_scope;
	std::vector<uint8_t> m_queue;
	std::vector<uint8_t> m_opcode;

	int toint(const std::string& s) const {
		int res = 0;
		sscanf(s.c_str(), "%d", &res);
		return res;
	}
/*
0 	CF 	Carry flag 	Status
1 		Reserved, always 1 in EFLAGS [2] 	 
2 	PF 	Parity flag 	Status
3 		Reserved 	 
4 	AF 	Adjust flag 	Status
5 		Reserved 	 
6 	ZF 	Zero flag 	Status
7 	SF 	Sign flag 	Status
8 	TF 	Trap flag (single step) 	Control
9 	IF 	Interrupt enable flag 	Control
10 	DF 	Direction flag 	Control
11 	OF 	Overflow flag 	Status
*/	void print_flags(uint32_t flags) {
		int w = 13, i;
		printf("[");
		if (flags & 0x001) { printf(" CF"); w-=3; }
		if (flags & 0x004) { printf(" PF"); w-=3; }
		if (flags & 0x010) { printf(" AF"); w-=3; }
		if (flags & 0x040) { printf(" ZF"); w-=3; }
		if (flags & 0x080) { printf(" SF"); w-=3; }
		if (flags & 0x100) { printf(" TF"); w-=3; }
		if (flags & 0x200) { printf(" IF"); w-=3; }
		if (flags & 0x400) { printf(" DF"); w-=3; }
		if (flags & 0x800) { printf(" OF"); w-=3; }
		printf(" ]");
		for (i = 0; i < w; i++) {
			printf(" ");
		}
	}
	void dump() {
#if 1
		if (m_donecount >= 4) {
			return;
		}
#endif
		uint32_t cs, ip, ds, es, ax, cx, si, di, flags;
		std::string current_state = m_values[m_syms["uut.cpu0.cpuproc.current_state"]];
		std::string next_state = m_values[m_syms["uut.cpu0.cpuproc.next_state"]];
		cs = toint(m_values[m_syms["uut.cpu0.cpudpath.cs_s[15:0]"]]);
		ip = toint(m_values[m_syms["uut.cpu0.cpudpath.ipreg[15:0]"]]);
		ds = toint(m_values[m_syms["uut.cpu0.cpudpath.ds_s[15:0]"]]);
		es = toint(m_values[m_syms["uut.cpu0.cpudpath.es_s[15:0]"]]);
		ax = toint(m_values[m_syms["uut.cpu0.cpudpath.ax_s[15:0]"]]);
		cx = toint(m_values[m_syms["uut.cpu0.cpudpath.cx_s[15:0]"]]);
		si = toint(m_values[m_syms["uut.cpu0.cpudpath.si_s[15:0]"]]);
		di = toint(m_values[m_syms["uut.cpu0.cpudpath.di_s[15:0]"]]);
		flags = toint(m_values[m_syms["uut.cpu0.cpudpath.ccbus[15:0]"]]);
		int clrop = toint(m_values[m_syms["uut.cpu0.clrop"]]);
		uint32_t pc = cs * 16 + ip;
		if (pc >= 0xffffc) {
//			printf("pc=%" PRIx32 ", donecount=%d\n", pc, m_donecount);
			m_donecount++;
		}
		if (current_state == "sdecode") {
			if (m_queue.size() > 0) {
#if 1
				if (m_opcode.size() == 0) {
//				printf("%3" PRIu64 ":", m_time / 1000000);
				printf("cs=%04" PRIx32 " ip=%04" PRIx32 " ", cs, ip);
				printf("ds=%04" PRIx32 " es=%04" PRIx32 " ", ds, es);
				printf("ax=%04" PRIx32 " cx=%04" PRIx32 " ", ax, cx);
				printf("si=%04" PRIx32 " di=%04" PRIx32 " ", si, di);
				printf("fl=%04" PRIx32 " ", flags);
				print_flags(flags);
				}
#endif
//				printf("queue len=%d\n", (int)m_queue.size());
				m_opcode.insert(m_opcode.end(), m_queue.begin(), m_queue.end());
				m_queue.clear();
			}
		} else {
//			printf("curr=%s next=%s\n", current_state.c_str(), next_state.c_str());
		}
//		uint32_t abus = toint(m_values[m_syms["uut.cpu0.abus[19:0]"]]);
		int latchabus = toint(m_values[m_syms["uut.cpu0.cpubiu.latchabus"]]);
		if (m_last_latchabus == 1 && latchabus == 0) {
//			printf("latch cur state %s\n", current_state.c_str());
//			printf("latch next state %s\n", next_state.c_str());
			uint8_t dbus_in = toint(m_values[m_syms["uut.cpu0.cpubiu.dbus_in[7:0]"]]);
			if (next_state == "sopcode") {
#if 0
				uint32_t abus = toint(m_values[m_syms["uut.cpu0.abus[19:0]"]]);
#if 0
				printf("cs=%04" PRIx32 " ip=%04" PRIx32 " ", cs, ip);
				printf("ds=%04" PRIx32 " es=%04" PRIx32 " ", ds, es);
				printf("ax=%04" PRIx32 " cx=%04" PRIx32 " ", ax, cx);
				printf("si=%04" PRIx32 " di=%04" PRIx32 " ", si, di);
				printf("fl=%04" PRIx32 " ", flags);
#endif
				printf("push queue %02x at abus=%x\n", (int)dbus_in, (int)abus);
#endif
				m_queue.push_back(dbus_in);
			}
		}
		m_last_latchabus = latchabus;
		if (m_last_clrop == 0 && clrop == 1) {
			if (m_time == 0)
				return;
//			printf("\n");
#if 0
			printf("%3" PRIu64 ":", m_time / 1000000);
			printf("cs=%04" PRIx32 " ip=%04" PRIx32 " ", cs, ip);
			printf("ds=%04" PRIx32 " es=%04" PRIx32 " ", ds, es);
			printf("ax=%04" PRIx32 " cx=%04" PRIx32 " ", ax, cx);
			printf("si=%04" PRIx32 " di=%04" PRIx32 " ", si, di);
			printf("fl=%04" PRIx32 " ", flags);
			print_flags(flags);
#endif
//			printf("next_state=%s", next_state.c_str());
#if 1
			for (auto& it : m_opcode) {
				printf(" %02" PRIx8, it);
			}
			m_opcode.clear();
//			m_opcode = m_queue;
//			m_queue.clear();
//			printf("\n");
#endif
#if 0
			for (auto& it : m_queue) {
				printf("%02" PRIx8 " ", it);
			}
			m_queue.clear();
//			printf("\n");
#endif
//			old_pc = pc;
#if 0
			for (auto& it : m_queue) {
				printf("%02" PRIx8 " ", it);
			}
			m_queue.clear();
//			printf("\n");
#endif
			printf("\n");
		}
		m_last_clrop = clrop;
	}
	std::string flatten(const std::vector<std::string>& path) const {
		std::string res;
		for (auto& it : path) {
			res += it + ".";
		}
		return res;
	}
public:
	MyVCDParser(const char *vcd_file) : VCDParser(vcd_file), m_time(0), m_donecount(0), m_last_clrop(0), m_last_latchabus(0) {}

	virtual void Scope(const std::string& scope) {
		m_scope.push_back(scope);
	}
	virtual void UpScope() {
		m_scope.pop_back();
	}
	virtual void Var(const std::string& sym, const std::string& _name) {
		std::string name = flatten(m_scope) + _name;
		m_names[sym] = name;
		m_syms[name] = sym;
		m_vsyms.push_back(sym);
	}
	virtual void EoE() {
	}
	virtual void SetTime(uint64_t time) {
		m_time = time;
		dump();
	}
	virtual void SetVal(const std::string& sym, const char *val) {
		std::string name = m_names[sym];
		m_values[sym] = val;
	}
	virtual void SetVal(const std::string& sym, int ival) {
		std::string val;
		char buf[1024];
		snprintf(buf, sizeof(buf), "%d", ival);
		SetVal(sym, buf);
	}
};

int main(int argc, char *argv[]) {
	const char *fname = "a.vcd";
	int arg = 1;
	if (arg < argc) {
		fname = argv[arg++];
	}
	MyVCDParser vcd(fname);
	vcd.Run();
	return 0;
}
