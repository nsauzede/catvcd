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

#ifndef VCDPARSER_H__
#define VCDPARSER_H__

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <string>

class VCDParser {
private:
	const char *m_vcd;
	FILE *m_in;
	bool m_eoe;

	int bin2int( char *s) {
		int i, val = 0;
		int n = strlen( s);
		for (i = 0; i < n; i++) {
			val += (s[i] - '0') << (n - i - 1);
		}
		return val;
	}

public:
	VCDParser(const char *vcd_file = 0)
	: m_vcd(vcd_file), m_in(0), m_eoe(false) {
		m_in = fopen(m_vcd, "rt");
	}
	~VCDParser() {
		if (m_in) {
			fclose(m_in);
		}
	}
	virtual void Scope(const std::string& scope) = 0;
	virtual void UpScope() = 0;
	virtual void Var(const std::string& sym, const std::string& name) = 0;
	virtual void EoE() = 0;
	virtual void SetTime(uint64_t time) = 0;
	virtual void SetVal(const std::string& sym, int val) = 0;
	virtual void SetVal(const std::string& sym, const char *val) = 0;
	virtual void Run() {
		while (!feof(m_in)) {
			char buf[1024];
			fgets(buf, sizeof(buf), m_in);
			if (strstr(buf, "$scope")) {
				if (m_eoe) {
					printf("Forbidden to enter scope after End Of Elaboration\n");
					exit(1);
				}
				char scope[1024];
				sscanf( buf, "%*s %*s %s", scope);
				Scope(scope);
			} else if (strstr(buf, "$upscope")) {
				if (m_eoe) {
					printf("Forbidden to leave scope after End Of Elaboration\n");
					exit(1);
				}
				UpScope();
			} else if (strstr(buf, "$var")) {
				if (m_eoe) {
					printf("Forbidden to add var after End Of Elaboration\n");
					exit(1);
				}
				char name[1024], title[1024];
				sscanf( buf, "%*s %*s %*s %s %s %*s", name, title);
				Var(name, title);
			} else if (strstr(buf, "$enddefinitions")) {
				if (m_eoe) {
					printf("Forbidden to have multiple End Of Elaboration\n");
					exit(1);
				}
				EoE();
				m_eoe = true;
			} else if (buf[0] == '#') {
				if (!m_eoe) {
					printf("Forbidden to set time before End Of Elaboration\n");
					exit(1);
				}
				uint64_t curtime = 0;
				sscanf(buf, "#%" SCNu64, &curtime);
				SetTime(curtime);
			} else if (m_eoe) {
				int val;
				char name[1024];
				char v[1024];
				bool ivalid = false;
				bool svalid = false;
				switch (buf[0]) {
					case 'b':
						sscanf( buf, "b%s %s", v, name);
						val = bin2int( v);
						ivalid = true;
						break;
					case '0':
					case '1':
						sscanf( buf, "%1d%s", &val, name);
						ivalid = true;
						break;
					case 's':
						sscanf( buf, "s%s %s", v, name);
						svalid = true;
						break;
					default:
						break;
				}
				if (ivalid) {
					SetVal(name, val);
				} else if (svalid) {
					SetVal(name, v);
				}
			}
		}
	}
};

#endif/*VCDPARSER_H__*/
