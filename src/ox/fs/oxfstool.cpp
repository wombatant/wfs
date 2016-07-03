/*
 * Copyright 2015 - 2016 gtalent2@gmail.com
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ox/std/strops.hpp>
#include <ox/fs/filesystem.hpp>

using namespace ox::fs;

const char *usage = "usage:\n"
"\toxfs format [16,32,64] <size> <path>\n"
"\toxfs read <FS file> <inode>\n"
"\toxfs write <FS file> <inode> <insertion file>";

char *loadFileBuff(const char *path, ::size_t *sizeOut = nullptr) {
	FILE *file = fopen(path, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		const auto size = ftell(file);
		rewind(file);
		auto buff = (char*) malloc(size);
		fread(buff, size, 1, file);
		fclose(file);
		if (sizeOut) {
			*sizeOut = size;
		}
		return buff;
	} else {
		return nullptr;
	}
}

int format(int argc, char **args) {
	printf("Creating file system...\n");
	auto err = 1;
	if (argc >= 5) {
		auto type = ox::std::atoi(args[2]);
		auto size = ox::std::atoi(args[3]);
		auto path = args[4];
		auto buff = (ox::std::uint8_t*) malloc(size);
		
		printf("Size: %d bytes\n", size);
		printf("Type: %d\n", type);

		// format
		switch (type) {
			case 16:
				FileStore16::format(buff, size, ox::fs::OxFS_16);
				break;
			case 32:
				FileStore32::format(buff, size, ox::fs::OxFS_32);
				break;
			case 64:
				FileStore64::format(buff, size, ox::fs::OxFS_64);
				break;
		}
		createFileSystem(buff);

		auto file = fopen(path, "wb");
		if (file) {
			fwrite(buff, size, 1, file);
			err = fclose(file);
			if (err) {
				printf("Could not write to file: %s.\n", path);
			}
		} else {
			printf("Could not open file: %s.\n", path);
		}

		if (err == 0) {
			printf("Created file system %s\n", path);
		}
	}

	return err;
}

int read(int argc, char **args) {
	auto err = 1;
	if (argc >= 4) {
		auto fsPath = args[2];
		auto inode = ox::std::atoi(args[3]);
		::size_t fsSize;
		ox::std::uint64_t fileSize;

		auto fsBuff = loadFileBuff(fsPath, &fsSize);
		
		if (fsBuff) {
			auto fs = createFileSystem(fsBuff);

			auto output = fs->read(inode, &fileSize);

			if (output) {
				fwrite(output, fileSize, 1, stdout);
				err = 0;
			}

			delete fs;
			delete fsBuff;
		} else {
			fprintf(stderr, "Could not open file: %s\n", fsPath);
		}
	}
	return err;
}

int write(int argc, char **args) {
	auto err = 0;
	if (argc >= 5) {
		auto fsPath = args[2];
		auto inode = ox::std::atoi(args[3]);
		auto srcPath = args[4];
		::size_t srcSize;

		auto fsFile = fopen(fsPath, "rb");
		if (fsFile) {
			fseek(fsFile, 0, SEEK_END);

			const auto fsSize = (::size_t) ftell(fsFile);
			rewind(fsFile);
			auto fsBuff = (char*) malloc(fsSize);
			fread(fsBuff, fsSize, 1, fsFile);
			fclose(fsFile);

			auto srcBuff = loadFileBuff(srcPath, &srcSize);
			if (srcBuff) {
				auto fs = createFileSystem(fsBuff);
				if (fs) {
					err |= fs->write(inode, srcBuff, srcSize);
				} else {
					fprintf(stderr, "Invalid file system.\n");
				}

				if (err) {
					fprintf(stderr, "Could not write to file system.\n");
				} else {
					fsFile = fopen(fsPath, "wb");

					if (fsFile) {
						err = fwrite(fsBuff, fsSize, 1, fsFile) != 1;
						err |= fclose(fsFile);
						if (err) {
							fprintf(stderr, "Could not write to file system file.\n");
						}
					} else {
						err = 1;
					}
				}
			} else {
				err = 1;
				fprintf(stderr, "Could not load source file.\n");
			}

			free(fsBuff);
			free(srcBuff);
		} else {
			fprintf(stderr, "Could not open file system\n");
		}
	}
	return err;
}

int main(int argc, char **args) {
	auto err = 0;
	if (argc > 1) {
		auto cmd = args[1];
		if (::strcmp(cmd, "format") == 0) {
			err = format(argc, args);
		} else if (::strcmp(cmd, "read") == 0) {
			err = read(argc, args);
		} else if (::strcmp(cmd, "write") == 0) {
			err = write(argc, args);
		} else if (::strcmp(cmd, "help") == 0) {
			printf("%s\n", usage);
		} else {
			printf("Command '%s' not recognized.\n", cmd);
			err = 1;
		}
	} else {
		printf("%s\n", usage);
	}
	return err;
}