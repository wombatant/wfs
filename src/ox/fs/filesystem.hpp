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

enum FsType {
	OxFS_16 = 1,
	OxFS_32 = 2,
	OxFS_64 = 3
};

struct FileStat {
	uint64_t inode;
	uint64_t size;
};

class FileSystem {
	public:
		virtual ~FileSystem() {};

		virtual int read(ox::std::uint64_t inode, void *buffer, ox::std::uint64_t size) = 0;

		virtual ox::std::uint8_t *read(ox::std::uint64_t inode, ox::std::uint64_t *size) = 0;

		virtual int remove(ox::std::uint64_t inode) = 0;

		virtual int write(ox::std::uint64_t inode, void *buffer, ox::std::uint64_t size) = 0;

		virtual FileStat stat(ox::std::uint64_t inode) = 0;
};

template<typename FileStore>
class FileSystemTemplate: public FileSystem {

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
		FileSystemTemplate(void *buff);

		int mkdir(const char *path);

		int read(const char *path, void *buffer);

		ox::std::uint8_t *read(ox::std::uint64_t inode, ox::std::uint64_t *size) override;

		int read(ox::std::uint64_t inode, void *buffer, ox::std::uint64_t size) override;

		int remove(ox::std::uint64_t inode);

		int write(ox::std::uint64_t inode, void *buffer, ox::std::uint64_t size) override;

		FileStat stat(const char *path);

		FileStat stat(ox::std::uint64_t inode) override;

		static ox::std::uint8_t *format(void *buffer, typename FileStore::FsSize_t size, bool useDirectories);
};

template<typename FileStore>
FileSystemTemplate<FileStore>::FileSystemTemplate(void *buff) {
	store = (FileStore*) buff;
}

template<typename FileStore>
typename FileStore::InodeId_t FileSystemTemplate<FileStore>::INODE_ROOT_DIR = 2;

template<typename FileStore>
int FileSystemTemplate<FileStore>::mkdir(const char *path) {
	return 0;
}

template<typename FileStore>
FileStat FileSystemTemplate<FileStore>::stat(const char *path) {
	FileStat stat;
	return stat;
}

template<typename FileStore>
FileStat FileSystemTemplate<FileStore>::stat(ox::std::uint64_t inode) {
	FileStat stat;
	auto s = store->stat(inode);
	stat.size = s.size;
	stat.inode = s.inodeId;
	return stat;
}

template<typename FileStore>
int FileSystemTemplate<FileStore>::read(ox::std::uint64_t inode, void *buffer, ox::std::uint64_t size) {
	auto err = 1;
	auto s = store->stat(inode);
	if (size == s.size) {
		err = store->read(inode, buffer, nullptr);
	}
	return err;
}

template<typename FileStore>
ox::std::uint8_t *FileSystemTemplate<FileStore>::read(ox::std::uint64_t inode, ox::std::uint64_t *size) {
	auto s = store->stat(inode);
	auto buff = new ox::std::uint8_t[s.size];
	if (size) {
		*size = s.size;
	}
	if (store->read(inode, buff, nullptr)) {
		delete []buff;
		buff = nullptr;
	}
	return buff;
}

template<typename FileStore>
int FileSystemTemplate<FileStore>::remove(ox::std::uint64_t inode) {
	return store->remove(inode);
}

template<typename FileStore>
int FileSystemTemplate<FileStore>::write(ox::std::uint64_t inode, void *buffer, ox::std::uint64_t size) {
	return store->write(inode, buffer, size);
}

template<typename FileStore>
ox::std::uint8_t *FileSystemTemplate<FileStore>::format(void *buffer, typename FileStore::FsSize_t size, bool useDirectories) {
	buffer = FileStore::format((ox::std::uint8_t*) buffer, size);

	if (buffer && useDirectories) {
		char dirBuff[sizeof(Directory) + sizeof(DirectoryEntry) + 2];
		auto *dir = (Directory*) dirBuff;
		dir->files();
	}

	return (ox::std::uint8_t*) buffer;
}

typedef FileSystemTemplate<FileStore16> FileSystem16;
typedef FileSystemTemplate<FileStore32> FileSystem32;
typedef FileSystemTemplate<FileStore64> FileSystem64;

FileSystem *createFileSystem(void *buff);

}
}
