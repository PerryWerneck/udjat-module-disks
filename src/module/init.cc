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
 #include <udjat/module.h>
 #include <udjat/factory.h>
 #include <blkid/blkid.h>
 #include <unistd.h>
 #include <agent.h>
 #include <fstream>
 #include <vector>
 #include <udjat/tools/file.h>
 #include <udjat/tools/xml.h>

 using namespace std;

 static const Udjat::ModuleInfo moduleinfo{
	PACKAGE_NAME,								// The module name.
	"Logical disk status monitor", 				// The module description.
	PACKAGE_VERSION, 							// The module version.
	PACKAGE_URL, 								// The package URL.
	PACKAGE_BUGREPORT 							// The bug report address.
 };

 class Module : public Udjat::Module, Udjat::Factory {
 public:

 	Module() : Udjat::Module("disk",&moduleinfo), Udjat::Factory("storage",&moduleinfo) {
 	};

 	virtual ~Module() {
 	}

	void parse(Udjat::Abstract::Agent &parent, const pugi::xml_node &node) const override {

		/// @brief Container with all disks
		class Container : public Udjat::Abstract::Agent {
		public:
			Container(const pugi::xml_node &node) : Udjat::Abstract::Agent("storage") {

				icon = "drive-multidisk";
				label = "Logical disks";
				load(node);

				//
				// Load devices.
				//
				struct List {

					struct Device {
						string devname;
						string label;
						string mountpoint;
						string type;

						Device(const char *d, const string &l, const string &t) : devname(d),label(l),type(t) {
						}

					};

					std::vector<Device> values;

					inline std::vector<Device>::iterator begin() {
						return values.begin();
					}

					inline std::vector<Device>::iterator end() {
						return values.end();
					}

				};

				List devices;

				//
				// Get block devices with labels.
				//
				{
					blkid_cache cache = NULL;

					blkid_get_cache(&cache,NULL);
					blkid_probe_all(cache);
					blkid_dev_iterate iter = blkid_dev_iterate_begin(cache);
					blkid_dev dev;
					while (blkid_dev_next(iter, &dev) == 0) {

						dev = blkid_verify(cache, dev);
						if (!dev)
							continue;

						string label;
						string type;

						blkid_tag_iterate tag = blkid_tag_iterate_begin(dev);
						const char *t, *value;
						while (blkid_tag_next(tag, &t, &value) == 0) {

							if(!strcasecmp(t,"LABEL")) {
								label = value;
								cout << "disk\tDetected device '" << blkid_dev_devname(dev) << "' with name '" << label << "'" << endl;
							} else if(!strcasecmp(t,"TYPE")) {
								type = value;
							}

						}
						blkid_tag_iterate_end(tag);
						devices.values.emplace_back(blkid_dev_devname(dev),label,type);

					}

					blkid_dev_iterate_end(iter);

					blkid_put_cache(cache);

				}

				// Get mount points
				{
					Udjat::File::Text mounts("/proc/mounts");

					for(auto device = devices.begin(); device != devices.end(); device++) {

						const char *devname = device->devname.c_str();
						size_t szName = device->devname.size();

						for(auto mp : mounts) {
							if(!strncmp(devname,mp->c_str(),szName)) {
								const char *from = mp->c_str()+szName;
								while(*from && isspace(*from))
									from++;
								const char *to = from;
								while(*to && !isspace(*to)) {
									to++;
								}
								if(*to) {
									device->mountpoint = string(from,to-from);
									cout	<< "disk\tUsing " << device->mountpoint
											<< " as mount point for " << device->devname
											<< " (" << device->label << ")"
											<< endl;
									break;
								}
							}
						}

					}
				}

				// Create agents
				{

					for(auto device = devices.begin(); device != devices.end(); device++) {

						if(device->mountpoint.empty()) {
							continue;
						}

						// Check for ignore-[type] attribute
						if(Udjat::Attribute(node,(string{"ignore-"} + device->type).c_str()).as_bool(false)) {
							cout << "disk\tIgnoring '" << device->mountpoint << "'" << endl;
							continue;
						}

						this->insert(
							std::make_shared<::Agent>(
								Udjat::Quark(device->mountpoint).c_str(),
								Udjat::Quark(device->label).c_str(),
								node,
								false
							)
						);

					}

				}


			}

			virtual ~Container() {
			}

			/// @brief Export info.
			void get(const Udjat::Request &request, Udjat::Response &response) override {

				Udjat::Abstract::Agent::get(request,response);

				if(request != Udjat::Request::Get)
					return;

				Json::Value devices(Json::arrayValue);

				for(auto child : *this) {

					auto agent = dynamic_cast<::Agent *>(child.get());
					if(!agent)
						continue;

					// It's an smart agent, export it.
					Json::Value device(Json::objectValue);

					device["name"] = agent->getName();
					device["summary"] = agent->getSummary();
					device["icon"] = agent->getIcon();
					device["state"] = agent->getState()->getSummary();
					device["level"] = std::to_string(agent->getState()->getLevel());
					device["used"] = agent->to_string();
					device["mp"] = agent->getMountPoint();

					devices.append(device);

				}

				response["disks"] = devices;

			}

		};

		const char * mountpoint = node.attribute("mount-point").as_string();

		if(*mountpoint) {

			// Has device name, create a device node.
			parent.insert(make_shared<Agent>(Udjat::Quark(mountpoint).c_str(),"",node,true));

		} else {

			// No device name, create a container with all detected devices.
			parent.insert(make_shared<Container>(node));

		}

	}

 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
	return new ::Module();
 }


