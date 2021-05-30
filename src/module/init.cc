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

						Device(const char *d, const char *l) : devname(d),label(l) {
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

						blkid_tag_iterate tag = blkid_tag_iterate_begin(dev);
						const char *type, *value;
						while (blkid_tag_next(tag, &type, &value) == 0) {

							if(!strcasecmp(type,"LABEL")) {
								label = value;
								cout << "disk\tDetected device '" << blkid_dev_devname(dev) << "' with name '" << label << "'" << endl;
							}

						}
						blkid_tag_iterate_end(tag);
						devices.values.emplace_back(blkid_dev_devname(dev),label.c_str());

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

						this->insert(
							std::make_shared<::Agent>(
								device->mountpoint.c_str(),
								device->label.c_str(),
								node,
								false
							)
						);

					}

				}


			}

			virtual ~Container() {
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


