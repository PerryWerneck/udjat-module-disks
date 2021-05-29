/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <agent.h>
 #include <udjat/tools/quark.h>
 #include <udjat/filesystem.h>
 #include <iostream>
 #include <sstream>
 #include <iomanip>

 using namespace std;

 // https://www.tecmint.com/linux-directory-structure-and-important-files-paths-explained/

 static const struct SysDef {
	const char *mp;
	const char *name;
	const char *icon;
	const char *label;
	const char *summary;
 } sysdefs[] = {
	{
		"/",
		"system",
		"drive-harddisk-system",
		"System root",
		""
	},

	// Home directory of the users
	{
		"/home",
		"home",
		"user-home",
		"User's homes",
		"Home directory of the users"
	},

	// All the executable binary programs (file) required during booting, repairing, files required to run into single-user-mode, and other important, basic commands viz., cat, du, df, tar, rpm, wc, history, etc.
	{
		"/bin",
		"bin",
		"applications-system",
		"Binary programs",
		""
	},

	// Holds important files during boot-up process, including Linux Kernel.
	{
		"/boot",
		"boot",
		"",
		"Boot-up process",
		""
	},

	// Contains device files for all the hardware devices on the machine e.g., cdrom, cpu, etc
	{
		"/dev",
		"dev",
		"",
		"Hardware devices",
		""
	},

	// Contains Application’s configuration files, startup, shutdown, start, stop script for every individual program.
	{
		"/etc",
		"etc",
		"",
		"Configuration files",
		""
	},

	// The Lib directory contains kernel modules and shared library images required to boot the system and run commands in root file system.
	{
		"/lib",
		"lib",
		"",
		"Kernel modules and library images",
		""
	},

	// Temporary mount directory is created for removable devices viz., media/cdrom.
	{
		"/media",
		"media",
		"drive-removable-media",
		"Removable devices",
		""
	},

	// Temporary mount directory for mounting file system.
	{
		"/mnt",
		"mnt",
		"",
		"Temporary mount",
		""
	},

	// Optional is abbreviated as opt. Contains third party application software. Viz., Java, etc.
	{
		"/opt",
		"opt",
		"",
		"Third party application",
		""
	},

	// A virtual and pseudo file-system which contains information about running process with a particular Process-id aka pid.
	{
		"/proc",
		"proc",
		"",
		"",
		""
	},

	// This is the home directory of root user and should never be confused with ‘/‘
	{
		"/root",
		"root",
		"user-home",
		"Root user home directory",
		""
	},

	// This directory is the only clean solution for early-runtime-dir problem.
	{
		"/run",
		"run",
		"",
		"",
		""
	},

	// Contains binary executable programs, required by System Administrator, for Maintenance. Viz., iptables, fdisk, ifconfig, swapon, reboot, etc.
	{
		"/sbin",
		"sbin",
		"",
		"Sysadmin binaries",
		""
	},

	// HTTP root
	{
		"/srv/www",
		"www",
		"folder-publicshare",
		"HTTP server files",
		""
	},

	// Service is abbreviated as ‘srv‘. This directory contains server specific and service related files.
	{
		"/srv",
		"srv",
		"",
		"Service related files",
		""
	},

	// Modern Linux distributions include a /sys directory as a virtual filesystem, which stores and allows modification of the devices connected to the system.
	{
		"/sys",
		"sys",
		"",
		"",
		""
	},

	// System’s Temporary Directory, Accessible by users and root. Stores temporary files for user and system, till next boot.
	{
		"/tmp",
		"tmp",
		"",
		"System temporary files",
		""
	},

	// Contains executable binaries, documentation, source code, libraries for second level program.
	{
		"/usr",
		"usr",
		"",
		"Second level programs",
		""
	},

	// Stands for variable. The contents of this file is expected to grow. This directory contains log, lock, spool, mail and temp files.
	{
		"/var",
		"var",
		"",
		"Variable files",
		""
	},

 };

 static const char * getNameFromMP(const char *mp, const char *name) {

	if(name && *name) {
		return name;
	}

	for(size_t ix = 0; ix < (sizeof(sysdefs)/sizeof(sysdefs[0])); ix++) {

		if(!strcasecmp(mp,sysdefs[ix].mp)) {
			return sysdefs[ix].name;
		}

	}

	const char *ptr = strrchr(mp,'/');
	if(ptr)
		return Udjat::Quark(ptr+1).c_str();

	return Udjat::Quark(mp).c_str();

 }

 Agent::Agent(const char * m, const char *name) : Udjat::Agent<float>(getNameFromMP(m,name)), mount_point(m) {
 	setup();
	setDefaultStates();
 }

 Agent::Agent(const char * m, const char *n, const pugi::xml_node &node, bool name_from_xml) : Udjat::Agent<float>(getNameFromMP(m,n)), mount_point(m) {
	setup();
	load(node,name_from_xml);
	if(!hasStates()) {
		setDefaultStates();
	}
 }

 void Agent::setup() {

	for(size_t ix = 0; ix < (sizeof(sysdefs)/sizeof(sysdefs[0])); ix++) {

		if(!strcasecmp(mount_point,sysdefs[ix].mp)) {

			// Have sysdef, update agent information.
			this->icon = sysdefs[ix].icon;
			this->label = sysdefs[ix].label;
			this->summary = sysdefs[ix].summary;
			break;

		}

	}

 }

 void Agent::refresh() {
 	set(Udjat::FileSystem(mount_point).used() * 100);
 }

 std::string Agent::to_string() const {

	// https://stackoverflow.com/questions/14432043/float-formatting-in-c
	std::stringstream out;
 	out << std::fixed << std::setprecision(2) << super::get() << "%";
 	return out.str();

 }

 void Agent::setDefaultStates() {

	static const struct {
		float from;
		float to;
		const char 						* name;			///< @brief State name.
		Udjat::Level					  level;		///< @brief State level.
		const char						* summary;		///< @brief State summary.
		const char						* body;			///< @brief State description
	} states[] = {
		{
			0.0,
			70.0,
			"good",
			Udjat::ready,
			"${agent.name} usage is less than 70%",
			""
		},
		{
			70.0,
			90.0,
			"gt70",
			Udjat::warning,
			"${agent.name} usage is greater than 70%",
			""
		},
		{
			90.0,
			98.0,
			"gt90",
			Udjat::error,
			"${agent.name} usage is greater than 90%",
			""
		},
		{
			98.0,
			100,
			"full",
			Udjat::error,
			"${agent.name} is full",
			""
		}
	};

	cout << this->getName() << "\tUsing default states" << endl;

	for(size_t ix = 0; ix < (sizeof(states)/ sizeof(states[0])); ix++) {

		string summary(states[ix].summary);
		string body(states[ix].body);

		expand(summary);
		expand(body);

		push_back(
			make_shared<Udjat::State<float>>(
				states[ix].name,
				states[ix].from,
				states[ix].to,
				states[ix].level,
				Udjat::Quark(summary).c_str(),
				Udjat::Quark(body).c_str()
			)
		);

	}

 }

 Agent::~Agent() {
 }

