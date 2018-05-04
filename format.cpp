#include <file_util.h>


int format_default_size(char* filename){
	int fd;
	fd = open(filename, O_RDWR | O_TRUNC | O_CREAT);
	if (fd == -1) {
		return EXIT_FAILURE;
	}
	ftruncate(fd, DEFAULT_SIZE);
	char *bootblock = malloc(BOOT_SIZE);
	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
	out = write(fd, bootblock, BOOT_SIZE);
	if (out != BOOT_SIZE) {
		return EXIT_FAILURE;
	}
	free(bootblock);
	sb = (Superblock*) malloc(sizeof(SuperBlock));
	sb->size = BLOCK_SIZE;
	sb->inode_offset = 0;
	sb->data_offset = DEFAULT_SIZE * INODE_RATE / BLOCK_SIZE;
	sb->free_inode = 0;
	sb->free_block = 0;

	num_of_total_data_block = (DEFAULT_SIZE - BOOT_SIZE - SUPER_SIZE - (sb->data_offset - sb->inode_offset) * BOOT_SIZE) / BOOT_SIZE;
	fseek(fd, BOOT_SIZE, SEEK_SET);
	write(fd, sb, SUPER_SIZE);

	for (int i = 0; i < num_of_total_data_block; i++) {
		if (i == num_of_total_data_block - 1) {
			int to_write = -1;
			fseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &to_write, sizeof(int));
		}
		else {
			fseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &i, sizeof(int));
		}
	}

	// construct a default inode
	inode* default_inode = (inode*) malloc(sizeof(inode));
	default_inode->nlink = 0;
	default_inode->permission = 7;
	default_inode->type = NORMAL_FILE;
	default_inode->next_inode = 1;
	default_inode->size = 0;
	default_inode->uid = 0;
	default_inode->gid = 0;
	for (int i = 0; i < N_DBLOCKS; i++) {
		default_inode->dblocks[i] = 0;
	}

	for (int i = 0; i < N_IBLOCKS; i++) {
		default_inode->iblocks[i] = 0;
	}

	default_inode->i2block = 0;
	default_inode->i3block = 0;
	default_inode->file_name = "";
	default_inode->padding = 0;

	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

	for (int i = 0; i < num_of_total_inode; ++i) {
		if (i == num_of_total_inode - 1) {
			default_inode->next_inode = -1;
			fseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
		else {
			default_inode->next_inode = i + 1;
			fseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
	}

	free(default_inode);
	close(fd);


}













