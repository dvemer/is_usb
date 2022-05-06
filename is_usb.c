#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>

#define SYS_BUS_PATH	"/sys/bus/usb/devices/"
#define SYS_CLASS_PATH	"/sys/class/block/"

static int is_symlink(const char *path, int *is_link)
{
	struct stat st;

	if (lstat(path, &st))
		return -1;

	*is_link = S_ISLNK(st.st_mode);

	return 0;
}

static int is_usb_drive(const char *name)
{
	DIR *dir;
	int is_link;
	struct dirent *entry;
	char link_dst[PATH_MAX] = { 0 };
	char sys_class_path[PATH_MAX] = { 0 };

	strcat(sys_class_path, SYS_CLASS_PATH);
	strcat(sys_class_path, name);

	if (access(sys_class_path, F_OK))
		return -1;

	if (is_symlink(sys_class_path, &is_link))
		return -1;

	if (!is_link)
		return -1;

	if (!realpath(sys_class_path, link_dst))
		return -1;

	dir = opendir(SYS_BUS_PATH);

	if (!dir)
		return -1;

	while ((entry = readdir(dir))) {
		char usb_dev_path[PATH_MAX];
		char usb_dev_path_dst[PATH_MAX] = { 0 };
		char link_dst_tmp[PATH_MAX];

		strcpy(usb_dev_path, SYS_BUS_PATH);
		strcat(usb_dev_path, entry->d_name);

		if (!strcmp(entry->d_name, ".") ||
		    !strcmp(entry->d_name, ".."))
			continue;

		if (is_symlink(usb_dev_path, &is_link))
			break;

		if (!is_link)
			continue;

		if (!realpath(usb_dev_path, usb_dev_path_dst))
			return -1;

		strcpy(link_dst_tmp, link_dst);

		while (1) {
			char *pos;

			if (!strcmp(link_dst_tmp, usb_dev_path_dst))
				return 0;

			pos = strrchr(link_dst_tmp, '/');

			if (!pos)
				break;

			*pos = '\0';
		}

	}

	closedir(dir);	

	return 1;
}

static int dump_all(void)
{
	char path[] = { 's', 'd', '\0', '\0', '\0' };

	for (char c = 'a'; c <= 'z';c++) {
		char idx;

		path[sizeof(path) - 2] = '\0';
		path[sizeof(path) - 3] = c;

		for (idx = '1';idx <= '9';idx++) {
			path[sizeof(path) - 2] = idx;

			if (!is_usb_drive(path))
				printf("%s ", path);
		}
	}
	
	fflush(stdout);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return -1;

	if (!strcmp(argv[1], "--all"))
		dump_all();
	else
		return is_usb_drive(argv[1]);
}
