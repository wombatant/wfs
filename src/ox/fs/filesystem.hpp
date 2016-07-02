/*
 * Copyright 2015 - 2016 gtalent2@gmail.com
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ox/std/std.hpp>
#include "filestore.hpp"

namespace ox {
namespace fs {

using namespace std;

struct FileStat {
	uint64_t inode;
	uint64_t size;
};

template<typename FileStore>
class FileSystem {

	private:
		struct DirectoryEntry {
			typename FileStore::InodeId_t inode;

			char *getName() {
				return (char*) (this + 1);
			}

			void setName(const char *name) {
				auto data = getName();
				auto nameLen = strlen(name);
				memcpy(data, &name, nameLen);
				data[nameLen] = 0;
			}
		};

		struct Directory {
			/**
			 * Number of files in this directory.
			 */
			typename FileStore::InodeId_t size = 0;

			DirectoryEntry *files() {
				return (DirectoryEntry*) (this + 1);
			}
		};

		// static members
		static typename FileStore::InodeId_t INODE_ROOT_DIR;

		FileStore *store = nullptr;

	public:
		int mkdir(const char *path);

		int read(const char *path, void *buffer);

		FileStat stat(const char *path);

		FileStat stat(typename FileStore::InodeId_t inode);

		static uint8_t *format(uint8_t *buffer, typename FileStore::FsSize_t size);
};

template<typename FileStore>
typename FileStore::InodeId_t FileSystem<FileStore>::INODE_ROOT_DIR = 2;

template<typename FileStore>
int FileSystem<FileStore>::mkdir(const char *path) {
	return 0;
}

template<typename FileStore>
FileStat FileSystem<FileStore>::stat(const char *path) {
	FileStat stat;
	return stat;
}

template<typename FileStore>
FileStat FileSystem<FileStore>::stat(typename FileStore::InodeId_t inode) {
	FileStat stat;
	auto s = store->stat(inode);
	stat.size = s.size;
	stat.inode = s.inodeId;
	return stat;
}

template<typename FileStore>
uint8_t *FileSystem<FileStore>::format(uint8_t *buffer, typename FileStore::FsSize_t size) {
	buffer = FileStore::format(buffer, size);
	char dirBuff[sizeof(Directory) + sizeof(DirectoryEntry) + 2];
	auto *dir = (Directory*) dirBuff;

	if (buffer) {
		dir->files();
	}

	return buffer;
}

typedef FileSystem<FileStore16> FileSystem16;
typedef FileSystem<FileStore32> FileSystem32;
typedef FileSystem<FileStore64> FileSystem64;

}
}