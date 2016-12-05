// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#ifndef TAR_READER_HPP
#define TAR_READER_HPP

#include <posix/tar.h>  // our posix header has the Tar_header struct, which the newlib tar.h does not

#include <string>
#include <vector>
#include <stdexcept>

// #include <gsl/gsl>
// #include <sys/stat.h>

extern char _binary_input_bin_start;
extern uintptr_t _binary_input_bin_size;

const int SECTOR_SIZE = 512;

class Tar_exception : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct Content_block {
  char block[SECTOR_SIZE];
};

class File_info {

public:
  File_info(Tar_header& header)
    : header_{header} {}

  File_info(Tar_header& header, const std::vector<Content_block*> content/*, int num_content_blocks*/)
    : header_{header}, content_{content}/*, num_content_blocks_{num_content_blocks}*/ {}

  const Tar_header& header() const { return header_; }
  void set_header(const Tar_header& header) { header_ = header; }

  const std::vector<Content_block*>& content() const { return content_; }
  void add_content_block(Content_block* content_block) {
    content_.push_back(content_block);
  }

  /*void set_num_content_blocks(int num_content_blocks) { num_content_blocks_ = num_content_blocks; }
  int num_content_blocks() const { return num_content_blocks_; }*/

  const std::string name() { return std::string{header_.name}; }
  const std::string mode() { return std::string{header_.mode}; }
  const std::string uid() { return std::string{header_.uid}; }
  const std::string gid() { return std::string{header_.gid}; }
  long int size();
  const std::string mod_time() { return std::string{header_.mod_time}; }
  const std::string checksum() { return std::string{header_.checksum}; }
  char typeflag() const { return header_.typeflag; }
  const std::string linkname() { return std::string{header_.linkname}; }
  const std::string magic() { return std::string{header_.magic}; }
  const std::string version() { return std::string{header_.version}; }
  const std::string uname() { return std::string{header_.uname}; }
  const std::string gname() { return std::string{header_.gname}; }
  const std::string devmajor() { return std::string{header_.devmajor}; }
  const std::string devminor() { return std::string{header_.devminor}; }
  const std::string prefix() { return std::string{header_.prefix}; }
  const std::string pad() { return std::string{header_.pad}; }

  bool is_ustar() const { return header_.magic == TMAGIC; }

  bool is_dir() const { return header_.typeflag == DIRTYPE; }

  bool typeflag_is_set() const { return header_.typeflag == ' '; }

  bool is_empty() { return content_.size() == 0; }

  int num_content_blocks() { return content_.size(); }

private:
  Tar_header& header_;
  std::vector<Content_block*> content_;
  // int num_content_blocks_{0};
};

class Tar {

  // using Span = gsl::span<Content_block>;
  // using Span_iterator = gsl::span<Content_block>::iterator;

  //using Span = gsl::span<File_info>;
  //using Span_iterator = gsl::span<File_info>::iterator;

public:

  Tar() = default;

  int num_elements() const { return files_.size(); }

  void add_file(const File_info& file) { files_.push_back(file); }
  const File_info file(const std::string& path) const;
  const std::vector<File_info>& files() const { return files_; }
  std::vector<std::string> file_names() const;

  /*const std::vector<Content_block*> content(const std::string& path) {
    for (auto file_info : files_) {
      if (file_info.name() == path) {
        printf("Found path\n");

        return std::vector<Content_block*>{content_.begin() + file_info.start_index(), content_.begin() + file_info.start_index() + file_info.num_content_blocks()};
      }
    }

    throw Tar_exception(std::string{"Path " + path + " doesn't exist"});
  }*/

  /*void add_content_block(Content_block* content_block) {
    content_.push_back(content_block);
  }*/

private:
  std::vector<File_info> files_;
  // std::vector<Content_block*> content_;

  /*
  Span content_;
  int next_available_{0};
  */

};  // class Tar

class Tar_reader {

public:

  Tar& read_binary_tar() {
    const char* bin_content  = &_binary_input_bin_start;
    const int   bin_size     = (intptr_t) &_binary_input_bin_size;

    return read(bin_content, bin_size);
  }

  /* Linux

  void read(const std::string& filename) {
    const std::string tar_suffix = ".tar";
    const std::string tar_gz_suffix = ".tar.gz";
    const std::string mender_suffix = ".mender";

    if (filename == "")
      throw Tar_exception("Invalid filename: Filename is empty");

    // Maybe:
    if (filename.substr(filename.size() - tar_suffix.size()) == tar_suffix) {
      read_tar(filename);
      return;
    } else if (filename.substr(filename.size() - mender_suffix.size()) == mender_suffix) {
      printf("Mender file\n");
      read_mender(filename);
      return;
    } else if (filename.substr(filename.size() - tar_gz_suffix.size()) == tar_gz_suffix) {
      read_tar_gz(filename);
      return;
    }

    throw Tar_exception("Invalid filename suffix. Has to be .tar, .mender or .tar.gz");
  }
  */

  /* IncludeOS

  Tar& read(const char* file, size_t size) {

    // For now:
    read_tar(file, size);
    return tar_;
  */
  /*
    const std::string tar_suffix = ".tar";
    const std::string tar_gz_suffix = ".tar.gz";
    const std::string mender_suffix = ".mender";

    if (filename == "")
      throw Tar_exception("Invalid filename: Filename is empty");

    // Maybe:
    if (filename.substr(filename.size() - tar_suffix.size()) == tar_suffix) {
      read_tar(filename);
      return;
    } else if (filename.substr(filename.size() - mender_suffix.size()) == mender_suffix) {
      printf("Mender file\n");
      read_mender(filename);
      return;
    } else if (filename.substr(filename.size() - tar_gz_suffix.size()) == tar_gz_suffix) {
      read_tar_gz(filename);
      return;
    }

    throw Tar_exception("Invalid filename suffix. Has to be .tar, .mender or .tar.gz");

  }*/

  /*Tar&*/
  void decompress(const char* file, size_t size) {
    // tar.gz



    //return tar_;
  }

  /*Tar& read_tar_gz(const char* file, size_t size) {
    printf("tar.gz\n");
*/
/*
    struct stat stat_tar;

    if (stat(filename.c_str(), &stat_tar) == -1)
      throw Tar_exception(std::string{"Unable to open tar file " + filename});

    printf("Filename: %s\n", filename.c_str());
    printf("Size of tar file (stat): %ld\n", stat_tar.st_size);
*/

/*  Throws exception
    if (stat_tar.st_size % SECTOR_SIZE not_eq 0)
      throw Tar_exception("Invalid size of tar file. Rest: ");
*/

/*
    // std::vector<char> buf(stat_tar.st_size);
    // auto* buf_head = buf.data();
    char buf[stat_tar.st_size];
    auto* buf_head = buf;

    std::ifstream tar_stream{filename}; // Load into memory
    auto read_bytes = tar_stream.read(buf_head, stat_tar.st_size).gcount();

    printf("-------------------------------------\n");
    printf("Read %d bytes from tar file\n", read_bytes);
    printf("So this tar.gz file consists of %d sectors\n", (read_bytes / SECTOR_SIZE));
*/
/*
    return tar_;
  }
*/

private:
  // If more than one tar file in the service: vector ?
  // Have a Tar (or many) as a private attribute to Tar_reader at all?
  Tar tar_;

  Tar& read(const char* file, size_t size);

/* ---------------- READ_TAR(const std::string& filename)

  void read_tar(const std::string& filename) {
    printf("tar: const std::string&\n");

    // Get tar over network f.ex. - just load to memory/ram and expect
    // Buffer
    // vector of Headers
    // span (gsl) of blocks
    // Ok to use new if close/delete
    // Ang. dekomprimering: Memdisk? Blir linket rett inn i servicen (med assembly - bakes inn på et bestemt sted). Hvis ikke qemu har støtte for det?

    // Test if file exists

    struct stat stat_tar;

    if (stat(filename.c_str(), &stat_tar) == -1)
      throw Tar_exception(std::string{"Unable to open tar file " + filename});

    printf("Filename: %s\n", filename.c_str());
    printf("Size of tar file (stat): %ld\n", stat_tar.st_size);

    if (stat_tar.st_size % SECTOR_SIZE not_eq 0)
      throw Tar_exception("Invalid size of tar file");

    // std::vector<char> buf(stat_tar.st_size);
    // auto* buf_head = buf.data();
    char buf[stat_tar.st_size];
    auto* buf_head = buf;

    std::ifstream tar_stream{filename}; // Load into memory
    auto read_bytes = tar_stream.read(buf_head, stat_tar.st_size).gcount();

    printf("-------------------------------------\n");
    printf("Read %d bytes from tar file\n", read_bytes);
    printf("So this tar file consists of %d sectors\n", (read_bytes / SECTOR_SIZE));

    // TODO Temp: To span (gsl) later (contiguous memory)
    std::vector<Content_block*> blocks;

    std::vector<Tar_header*> headers;

    // Go through the whole buffer/tar file block by block
    for ( long i = 0; i < stat_tar.st_size; i += SECTOR_SIZE) {

      // One header or content at a time (one block)

      printf("----------------- New header -------------------\n");

      Tar_header* h = (Tar_header*) (buf + i);

      if (strcmp(h->name, "") == 0) {
        printf("Empty header name: Continue\n");
        continue;
      }

      // if h->name is a .tar.gz-file do something else (as is the case with mender)

      //strcpy(h->name, std::string{buf.begin() + i + OFFSET_NAME, buf.begin() + i + OFFSET_NAME + LENGTH_NAME}.c_str());

      printf("Name: %s\n", h->name);
      printf("Mode: %s\n", h->mode);
      printf("UID: %s\n", h->uid);
      printf("GID: %s\n", h->gid);
      printf("Filesize: %s\n", h->size);
      printf("Mod_time: %s\n", h->mod_time);
      printf("Checksum: %s\n", h->checksum);
      printf("Linkname: %s\n", h->linkname);
      printf("Magic: %s\n", h->magic);
      printf("Version: %s\n", h->version);
      printf("Uname: %s\n", h->uname);
      printf("Gname: %s\n", h->gname);
      printf("Devmajor: %s\n", h->devmajor);
      printf("Devminor: %s\n", h->devminor);
      printf("Prefix: %s\n", h->prefix);
      printf("Pad: %s\n", h->pad);
      printf("Typeflag: %c\n", h->typeflag);

      // Check if this is a directory or not (typeflag) (Directories have no content)

      if (h->typeflag not_eq DIRTYPE) {  // Is not a directory so has content

        // Next block is first content block and we need to subtract number of headers (are incl. in i)
        //h->first_block_index = (i / SECTOR_SIZE) - headers.size();
        //printf("First block index: %d\n", h->first_block_index);

        // Get the size of this file in the tarball
        char* pEnd;
        long int filesize;
        filesize = strtol(h->size, &pEnd, 8); // h->size is octal
        printf("Filesize decimal: %ld\n", filesize);

        int num_content_blocks = 0;
        if (filesize % SECTOR_SIZE not_eq 0)
          num_content_blocks = (filesize / SECTOR_SIZE) + 1;
        else
          num_content_blocks = (filesize / SECTOR_SIZE);
        printf("Num content blocks: %d\n", num_content_blocks);

        //h->num_content_blocks = num_content_blocks;

        for (int j = 0; j < num_content_blocks; j++) {

          i += SECTOR_SIZE; // Go to next block with content

          // TODO (not + 8 but doesn't print out anything many places if just buf + i)
          Content_block* content_blk = (Content_block*) (buf + i + 8);
          //Content_block content_blk;
          //snprintf(content_blk.block, SECTOR_SIZE, "%s", (buf + i));
          //printf("For-loop Content block: %s\n", content_blk.block);
          printf("\nContent block:\n%.512s\n", content_blk->block);

          // NB: Doesn't stop at 512 without %.512s - writes the rest as well (until the file's end)

          blocks.push_back(content_blk);  // TODO: GSL
        }
      }

      printf("After for-loop with content: %u content blocks\n", blocks.size());
      headers.push_back(h);
    }

    printf("Num headers: %u\n", headers.size());
    printf("Num content blocks: %u\n", blocks.size());

    tar_stream.close();
*/

/* Mender
    // START CONTENT of info-file:
    printf("Content: ");
    for (int i = 512; i < 1024; i++)
      printf("%s", std::string{buf[i]}.c_str());
    printf("\n");

    // Next file (header.tar.gz):

    printf("--------------------------------------\n");

    printf("Header.tar.gz (compressed):\n");

    Header h2;

    std::string h2_name{buf.begin() + 1024, buf.begin() + (1024 + 100)};

    printf("Name: ");
    for (int i = 1024; i < 1124; i++) {
      printf("%s", std::string{buf[i]}.c_str());
    }
    printf("\n");

    h2.set_name(h2_name);
    printf("H2's name: %s\n", h2.name().c_str());

    // osv.

    // header.tar.gz's content is from 1536 to 2048

    // Next file (data):

    printf("--------------------------------------\n");

    // Data is a directory containing image files (one compressed tar file for each image)
    printf("Data:\n");

    Header h3;

    std::string h3_name{buf.begin() + 2048, buf.begin() + (2048 + 100)};

    h3.set_name(h3_name);

    printf("Name: %s\n", h3.name().c_str());

    // osv.
*/
    /*printf("Content: ");
    for (int i = 2560; i < 3072; i++) {
      printf("%s");
    }*/

    // Terminate

    // fclose(f);
//}

/* A Reader provides sequential access to the contents of a tar archive. A tar archive consists of
// a sequence of files. The next method advances to the next file in the archive (including the first),
// and then it can be treated as an io.Reader to access the file's data.
class Reader {
	// Contains filtered or unexported fields

public:
	Reader((io.Reader) r);
	Reader next(const Reader& tr);				// Advances to the next entry in the tar archive (io.EOF is returned at the end of the input)
	int read(const char* bytes);		// Reads from the current entry in the tar archive. It returns 0, io.EOF when it reaches the end of that entry,
										// until next() is called to advance to the next entry
										// Calling read() on special types like TypeLink, TypeSymLink, TypeChar, TypeBlock, TypeDir, and TypeFifo returns
										// 0, io.EOF regardless of what the Header.size claims

private:
  int64_t pad;    // Amount of padding (ignored) after current file entry
  char blk[];     // Buffer to use as temporary local storage
};

// A Writer provides sequential writing of a tar archive in POSIX.1 format. A tar archive consists of a sequence of files.
// Call write_header() to begin a new file, and then call write() to supply that file's data, writing at most hdr.size bytes in total.
struct Writer {
	// Contains filtered or unexported fields

	Writer((io.Writer) w);
	void close();							// Closes the tar archive, flushing any unwritten data to the underlying writer
	void flush();							// Finishes writing the current file (optional)
	int write(const char* bytes);			// Writes to the current entry in the tar archive. Returns the error ErrWriteTooLong if more than
											// hdr.size bytes are written after write_header()
	void write_header(const Header& hdr);	// Writes hdr and prepares to accept the file's contents. Calls flush() if it is not the first header
											// Calling after a close() will return ErrWriteAfterClose
};
*/


/* -------------------- libarchive -------------------- */
/*
struct tar {
	struct archive_string acl_text;
	struct archive_string entry_pathname;
	struct
};*/

/*static int archive_read_format_tar_read_data(const Archive_read& a, const void** buff, size_t size, int64_t offset) {


}*/
/*
static void read_tar_2(const char* filename) {

	struct archive_entry* ae;
	struct archive *a;

	a = archive_read_new();

	if (a != NULL) {

		// Fill/Create Header
		Header hdr{file_info, link};

    // ...

		archive_read_next_header(a, &ae);

		archive_read_close(a);

		archive_read_free(a);
	}
}*/


/* From libarchive examples */

/*
#include <sys/types.h>		// POSIX - IncludeOS option?
#include <sys/stat.h>		// POSIX - IncludeOS option?

#include <archive.h>		// libarchive - implement
#include <archive_entry.h>	// libarchive - implement

#include <fcntl.h>			// POSIX
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>			// POSIX

static int verbose = 0;

// C -> C++

static void create(const char* filename, int compress, const char** argv) {
	struct archive* a;
	struct archive_entry* entry;
	ssize_t len;
	int fd;

	a = archive_write_new();							// IMPLEMENT

	archive_write_add_filter_none(a);					// IMPLEMENT - necessary?

	if (filename not_eq NULL and strcmp(filename, "-") == 0)
		filename = NULL;

	archive_write_open_filename(a, filename);			// IMPLEMENT

	while (*argv not_eq NULL) {
		struct archive* disk = archive_read_disk_new();	// IMPLEMENT

		// archive_read_disk_set_standard_lookup(disk);

		int r = archive_read_disk_open(disk, *argv);	// IMPLEMENT

		if (r not_eq ARCHIVE_OK) {
			errmsg(archive_error_string(disk));
			errmsg("\n");
			exit(1);
		}

		while (true) {
			int needcr = 0;

			entry = archive_entry_new();				// IMPLEMENT
			r = archive_read_next_header2(disk, entry);	// IMPLEMENT

			if (r == ARCHIVE_EOF)
				break;

			if (r not_eq ARCHIVE_OK) {
				errmsg(archive_error_string(disk));
				errmsg("\n");
				exit(1);
			}

			archive_read_disk_descend(disk);		// IMPLEMENT

			if (verbose) {
				msg("a ");
				msg(archive_entry_pathname(entry));
				needcr = 1;
			}

			r = archive_write_header(a, entry);

			if (r < ARCHIVE_OK) {
				errmsg(": ");
				errrmsg(archive_error_string(a));
				needcr = 1;
			}

			if (r == ARCHIVE_FATAL)					// ADD CONSTANT
				exit(1);

			if (r > ARCHIVE_FAILED) {

				// Copy data into the target archive

				fd = open(archive_entry_sourcepath(entry), O_RDONLY);	// Where?	// IMPLEMENT
				len = read(fd, buff, sizeof(buff));						// Where?

				while(len > 0) {
					archive_write_data(a, buff, len);	// IMPLEMENT
					len = read(fd, buff, sizeof(buff));
				}

				close(fd);
			}

			archive_entry_free(entry);					// IMPLEMENT

			if (needcr)
				msg("\n");
		}

		archive_read_close(disk);
		archive_read_free(disk);
		argv++;
	}

	archive_write_close(a);
	archive_write_free(a);
}

static void extract(const char* filename, int do_extract, int flags) {
	struct archive* a;
	struct archive* ext;
	struct archive_entry* entry;
	int r;

	a = archive_read_new();                      // IMPLEMENT
	ext = archive_write_disk_new();              // IMPLEMENT
	archive_write_disk_set_options(ext, flags);  // IMPLEMENT

	// tar:
	archive_read_support_format_tar(a);          // IMPLEMENT

	if (filename != NULL and strcmp(filename, "-") == 0)
		filename = NULL;

	if (r = archive_read_open_filename(a, filename, 10240)) {	// IMPLEMENT
		errmsg(archive_error_string(a));						// IMPLEMENT
		errmsg("\n");
		exit(r);												// In cstdlib
	}

	while (true) {
		int needcr = 0;
		r = archive_read_next_header(a, &entry);	// IMPLEMENT

		if (r == ARCHIVE_EOF)               // ADD CONSTANT
			break;

		if (r not_eq ARCHIVE_OK) {          // ADD CONSTANT
			errmsg(archive_error_string(a));
			errmsg("\n");
			exit(1);
		}

		if (verbose and do_extract)
			msg("x ");

		if (verbose or (not do_extract)) {
			msg(archive_entry_pathname(entry));		// IMPLEMENT
			msg(" ");
			needcr = 1;
		}

		if (do_extract) {
			r = archive_write_header(ext, entry);	// IMPLEMENT

			if (r not_eq ARCHIVE_OK) {
				errmsg(archive_error_string(a));
				needcr = 1;
			} else {
				r = copy_data(a, ext);				// IMPLEMENT

				if (r not_eq ARCHIVE_OK)
					needcr = 1;
			}
		}

		if (needcr)
			msg("\n");
	}

	archive_read_close(a);						// IMPLEMENT
	archive_read_free(a);						  // IMPLEMENT
	exit(0);
}

static int copy_data(struct archive* ar, struct archive* aw) {
	int r;
	const void* buff;
	size_t size;
	int64_t offset;

	while (true) {
		r = archive_read_data_block(ar, &buff, &size, &offset);		// IMPLEMENT

		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);

		if (r not_eq ARCHIVE_OK) {
			errmsg(archive_error_string(ar));
			return (r);
		}

		r = archive_write_data_block(aw, buff, size, offset);		// IMPLEMENT

		if (r not_eq ARCHIVE_OK) {
			errmsg(archive_error_string(ar));
			return (r);
		}
	}
}

static void msg(const char* m) {
	write(1, m, strlen(m));				// where? stat?
}

static void errmsg(const char* m) {
	if (m == NULL)
		m = "Error: No error description provided.\n";

	write(2, m, strlen(m));
}

// static void usage();

*/

};  // < class Tar_reader

#endif
