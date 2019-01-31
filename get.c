/**
 * Date: 02/04/2019
 * Class: CS5750
 * Assignment: Assignment 1
 * Author: Ahmed Radwan
 */
#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

void add_forward_slash(char **dst, int *dst_len) {
	if (strncmp((*dst) + (*dst_len) - 1, "/", 1) != 0) {
		(*dst) = realloc((*dst), (*dst_len) + 2);
		assert((*dst) != NULL);
		assert(strncat((*dst), "/", 1) != NULL);
		(*dst_len) += 1;
	}
}

void append_filename_to_destination(char **src, int src_len,
		char **dst, int dst_len) {

	char *token;
	char *filename;
	char *delim = "/";

	int filename_len;

	token = strtok((*src), delim);
	assert(token != NULL && "token is NULL!");
	while (token != NULL) {
		filename = token;
		token = strtok(NULL, delim);
	}
	filename_len = strlen(filename);
	assert(filename_len > 0 && "filename_len is NULL!");
	(*dst) = realloc((*dst), dst_len + filename_len + 1);
	assert((*dst) != NULL && "dst is NULL!");
	assert(strncat((*dst), filename, filename_len) != NULL && "strncat failed!");
}

void check_file_permissions(char *src, int perms[]) {
	struct stat file_stat;

	assert(lstat(src, &file_stat) != -1 && "check_file_permissions lstat failed!");
	
	// Set permissions
	if (file_stat.st_mode & S_IRUSR) perms[0] = 1;
	if (file_stat.st_mode & S_IWUSR) perms[1] = 1;
	if (file_stat.st_mode & S_IXUSR) perms[2] = 1;
	if (file_stat.st_mode & S_IRGRP) perms[3] = 1;
	if (file_stat.st_mode & S_IWGRP) perms[4] = 1;
	if (file_stat.st_mode & S_IXGRP) perms[5] = 1;
	if (file_stat.st_mode & S_IROTH) perms[6] = 1;
	if (file_stat.st_mode & S_IWOTH) perms[7] = 1;
	if (file_stat.st_mode & S_IXOTH) perms[8] = 1;

}

void check_file_size(FILE **ptr, long *size) {
	// Get file size
	assert(fseek((*ptr), 0, SEEK_END) != -1 && "fseek failed!");
	(*size) = ftell((*ptr));
	assert((*size) > 0 && "file is empty!");

	// Reset file_pointer to beginning
	assert(fseek((*ptr), 0, SEEK_SET) != -1 && "fseek failed!");
}

void check_if_acl_has_group_or_world_permissions(char *filename, int perms[]) {
	int i;

	// Group permissions
	for (i = 3; i < 6; i++) {
		if (perms[i] == 1) {
			printf("%s has group permissions set!\n", filename);
			exit(-1);
		}
	}

	// World permissions
	for (i = 6; i < 9; i++) {
		if (perms[i] == 1) {
			printf("%s has world permissions set!\n", filename);
			exit(-1);
		}
	}
}

// TODO
void check_if_acl_is_malformed(char *string) {

}

void check_if_full_path(char *path) {
	if (strncmp(path, "/", 1) != 0) {
		printf("%s is not a full path. (Does not start with /)\n", path);
		exit(-1);
	}
}

void check_if_owner_has_read_permissions(int perms[]) {
	uid_t uid, euid, suid;
	struct passwd *pw;

	// Get owner name
	assert(getresuid(&uid, &euid, &suid) != -1 && "getresuid() failed!");
	pw = getpwuid(suid);

	// Check if owner has read permissions
	if (perms[0] == 0) {
		printf("%s does not have read permissions!\n", pw->pw_name);
		exit(-1);
	}
}

void check_if_owner_owns_file(char *path) {
	uid_t uid, euid, suid;
	gid_t gid, egid, sgid;
	char *suid_name, *file_owner;
	char *sgid_group, *file_group;
	int suid_name_len, sgid_group_len, file_owner_len, file_group_len, length;
	struct passwd *pw;
	struct group *gr;
	struct stat file_stat;

	// GETRESUID & GETRESGID
	assert(getresuid(&uid, &euid, &suid) != -1 && "getresuid() failed!");
	assert(getresgid(&gid, &egid, &sgid) != -1 && "getresgid() failed!");

	// Get ID's associated username
	pw = getpwuid(suid);
	assert(pw != NULL && "passwd struct (suid) is NULL!");

	// Store suid's username
	suid_name_len = strlen(pw->pw_name);
	assert(suid_name_len > 0 && "length is not > 0!");
	suid_name = malloc(suid_name_len + 1);
	assert(suid_name != NULL && "suid_name malloc is NULL!");
	assert(strncpy(suid_name, pw->pw_name, suid_name_len) != NULL && "strncpy on suid_name is NULL!");

	// Get sgid's group name
	gr = getgrgid(sgid);
	assert(gr != NULL && "group struct (sgid) is NULL!");

	// Store sgid's group name
	sgid_group_len = strlen(gr->gr_name);
	assert(sgid_group_len > 0 && "length is not > 0!");
	sgid_group = malloc(sgid_group_len + 1);
	assert(sgid_group != NULL && "sgid_group malloc is NULL!");
	assert(strncpy(sgid_group, gr->gr_name, sgid_group_len) != NULL && "strncpy on sgid_group is NULL!");

	// fstat the file
	assert(lstat(path, &file_stat) != -1 && "lstat in \"chech_if_owner_owns_files\" failed!");

	// Get file's associated user
	pw = getpwuid(file_stat.st_uid);
	assert(pw != NULL && "passwd struct (st_uid) is NULL!");

	// Store file's owner
	file_owner_len = strlen(pw->pw_name);
	assert(file_owner_len > 0 && "length is not > 0!");
	file_owner = malloc(file_owner_len + 1);
	assert(file_owner != NULL && "file_owner malloc is NULL!");
	assert(strncpy(file_owner, pw->pw_name, file_owner_len) != NULL && "strncpy on file_owner is NULL!");

	// Get the file's associated group
	gr = getgrgid(file_stat.st_gid);
	assert(gr != NULL && "passwd struct (st_gid) is NULL!");

	// Store the file's group
	file_group_len = strlen(gr->gr_name);
	assert(file_group_len > 0 && "length is not > 0!");
	file_group = malloc(file_group_len + 1);
	assert(sgid_group != NULL && "file_group malloc is NULL!");
	assert(strncpy(file_group, gr->gr_name, file_group_len) != NULL && "strncpy on file_group is NULL!");

	/* UNCOMMENT TO TEST VALUES *
		printf("Owner  name: %s\n", suid_name);
		printf("Owner group: %s\n", sgid_group);
		printf("File  owner: %s\n", file_owner);
		printf("File  group: %s\n", file_group);
	*/

	// Find the lesser length
	if (suid_name_len < file_owner_len)
		length = suid_name_len;
	else
		length = file_owner_len;

	// Compare owner's name with the file owner's name
	if (strncmp(suid_name, file_owner, length) != 0) {
		printf("%s does not own %s\n", suid_name, path);
		exit(-1);
	}
}

void check_if_path_exists(char *path) {
	struct stat file_stat;

	if (lstat(path, &file_stat) != 0) {
		printf("%s does not exist!\n", path);
		exit(-1);
	}
}

void check_if_symbolic_link(char *path, char *path_type) {
	struct stat file_stat;

	assert(lstat(path, &file_stat) != -1);
	if (S_ISLNK(file_stat.st_mode)) {
		printf("%s is a symbolic link.\n", path_type);
		exit(-1);
	}
}

void check_if_user_allowed_to_read(char *string) {
	uid_t uid;
	char *token;
	char *delim = ",";
	int uid_len, tok_len, length, found;
	struct passwd *pw;

	uid = geteuid(); 		// Claims to always be successful
	pw = getpwuid(uid);	// Claims to always be successful

	uid_len = strlen(pw->pw_name);
	assert(uid_len > 0 && "uid_len is not > 0!");

	found = 0;

	token = strtok(string, delim);
	assert(token != NULL && "Access denied: user was not found in .acl file.");
	while (token != NULL) {
		tok_len = strlen(token);
		assert(tok_len > 0 && "tok_len is not > 0!");

		if (uid_len < tok_len)
			length = uid_len;
		else
			length = tok_len;

		if (strncmp(token, pw->pw_name, length) == 0) {
			found = 1;
		}

		token = strtok(NULL, delim);
	}

	if (!found) {
		printf("%s was not found in the acl file.\n", pw->pw_name);
		exit(-1);
	}

}

void create_acl_path(char *src, int src_len, char **acl, int *acl_len) {
	(*acl) = malloc(src_len + 5);
	assert(strncpy((*acl), src, src_len) != NULL);
	assert(strncat((*acl), ".acl", 4) != NULL);
	(*acl_len) = src_len + 4;
}

void make_copy_from_argv(char **path, int *path_len, char **argv, int param) {
	(*path_len) = strlen(argv[param]);
	assert((*path_len) > 0);
	(*path) = malloc((*path_len) + 1);
	assert((*path) != NULL);
	strncpy((*path), argv[param], (*path_len));
}

void open_file(char *pathname, char *mode, FILE **fptr) {
	(*fptr) = fopen(pathname, mode);
	assert((*fptr) != NULL && "File does not exist!");
}

void read_file(FILE **ptr, long *size, char **str) {
	(*str) = malloc((*size) + 1);
	assert((*str) != NULL && "str malloc failed!");
	assert(fread((*str), (*size), 1, (*ptr)) != 0 && "fread failed!");
}

int main(int argc, char **argv) {
	FILE *fptr;
	FILE *aclptr;
	char *src;
	char *dst;
	char *acl_path;
	char *read_string;
	char *acl_string;
	int src_len;
	int dst_len;
	int acl_path_len;
	long fsize;
	long aclsize;

	int src_perms[9] = {0};
	int acl_perms[9] = {0};

	// Check that both src and dst are provided
	if (argc != 3)
		return -1;

	// Copy source from argv
	make_copy_from_argv(&src, &src_len, argv, 1);

	// Copy destination from argv
	make_copy_from_argv(&dst, &dst_len, argv, 2);

	// Check if source file exists
	check_if_path_exists(src);

	// Check if source is symbolic link
	check_if_symbolic_link(src, "Source");

	// Check if owner owns the srouce file
	check_if_owner_owns_file(src);

	// Check if owner has read permissions
	check_file_permissions(src, src_perms);
	check_if_owner_has_read_permissions(src_perms);

	// Check if acl file exists
	create_acl_path(src, src_len, &acl_path, &acl_path_len);
	check_if_path_exists(acl_path);

	// Check if owner owns the acl file
	check_if_owner_owns_file(acl_path);

	// Check if acl has group or world permissions set
	check_file_permissions(acl_path, acl_perms);
	check_if_acl_has_group_or_world_permissions(acl_path, acl_perms);

	// Open acl file
	open_file(acl_path, "r", &aclptr);

	// Get file size
	check_file_size(&aclptr, &aclsize);

	// Read from the acl file
	read_file(&aclptr, &aclsize, &acl_string);

	// Check if acl is malformed
	check_if_acl_is_malformed(acl_string);

	// Check if user is in acl
	check_if_user_allowed_to_read(acl_string);

	// Open source file
	open_file(src, "r", &fptr);

	// Get file size
	check_file_size(&fptr, &fsize);

	// Read from source file
	read_file(&fptr, &fsize, &read_string);

	// Close source file;
	assert(fclose(fptr) == 0 && "fclose failed!");

	// Check if source file exists
	check_if_path_exists(dst);

	// Check if destination is symbolic link
	check_if_symbolic_link(dst, "Destination");

	// Check if destination path ends with "/"
	add_forward_slash(&dst, &dst_len);

	// Append filename to destination path
	append_filename_to_destination(&src, src_len, &dst, dst_len);

	// Open destination file
	open_file(dst, "w", &fptr);

	// Write to file
	assert(fwrite(read_string, fsize, 1, fptr) != 0);

	// Close destination file
	assert(fclose(fptr) == 0);

	printf("No errors!\n");

	return 0;
}