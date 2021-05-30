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

 #include <udjat.h>
 #include <udjat/module.h>
 #include <udjat/filesystem.h>
 #include <unistd.h>
 #include <agent.h>

 using namespace std;
 using namespace Udjat;

 /*
 static void test_filesystem() {

	::Agent agent("/");
	agent.refresh();

	cout << "Usage of '" << agent.getName() << "' is " << agent.to_string() << endl;

 }
 */

 static void agent_test() {

	for(auto agent : *Abstract::Agent::init("${PWD}/test.xml")) {
		cout << "http://localhost:8989/api/1.0/agent/" << agent->getName() << endl;
	}

	cout << "Waiting for requests" << endl;
	// Udjat::run();

	Abstract::Agent::deinit();

 }

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	Logger::redirect(nullptr,true);

	Module::load("http");
	auto module = udjat_module_init();

	agent_test();

	cout << "Removing module" << endl;
	delete module;
	Module::unload();

	return 0;
}
