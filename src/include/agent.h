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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/agent.h>
 #include <pugixml.hpp>

 class UDJAT_API Agent : public Udjat::Agent<float> {
 private:

	/// @brief Device name.
	const char *mount_point;

	void setup();

 public:
 	typedef Udjat::Agent<float> super;

	Agent(const char * mount_point, const char *name = "");
	Agent(const char * mount_point, const char *name, const pugi::xml_node &node);

	void start() override;

	/// @brief Get device status, update internal state.
	bool refresh() override;

	/// @brief Get mount point.
	inline const char * getMountPoint() const noexcept {
		return mount_point;
	}

	/// @brief Get value as string.
	std::string to_string() const override;

	virtual ~Agent();

 };

