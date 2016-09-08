#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "nv_wr.h"

typedef enum {
	VER_EMPTY = 0,
	VER_FLOAT,
	VER_INT,
} vType_t;

struct gen_args {
	float	ver_f;
	int32_t	ver_i;
	vType_t type;

	char	*file;
	int	reverseFlag;
	int	reverse;

	int	blockSize;
	int	blockCount;
	unsigned char bat_flag[3];
};

static struct gen_args args;

const char usage_str[] = {
	"Usage: nv_gen <-e version-float>|<-i version-int> <-f output-file>\n"
	"              [-b block-size -c block-count] [-r reverse-value]\n"
	"  or : nv_gen -h\n\n"
	"Option:\n"
	"  -e:    Version of target nv, version type float\n"
	"  -i:    Version of target nv, version type integer\n"
	"  -f:    Output file path\n"
	"  -b:    Output Block size(unit: Bytes).\n"
	"  -c:    Output Block count\n"
	"  -r:    Reverse Value for fill reverve space(Base on 8, 10, 16).\n"
	"  -x:    Set battery flag to nv\n"
	"  -h:    Help usage\n"
};

static void usage(void)
{
	printf(usage_str);
}

static char *appName = NULL;

static int parse_argument(struct gen_args *args, int argc, char **argv)
{
	int op;

	/* No argument */
	if (argc < 2)
		return -1;

	while ((op = getopt(argc, argv, "e:i:f:b:c:r:xh")) != -1) {
		switch (op) {
		case 'e':
			if (!args->type) {
				args->ver_f	= strtof(optarg, NULL);
				args->type	= VER_FLOAT;
			} else {
				printf("%s two version type at same times.\n", appName);
				return -1;
			}
			break;
		case 'i':
			if (!args->type) {
				args->ver_i	= atoi(optarg);
				args->type	= VER_INT;
			} else {
				printf("%s two version type at same times.\n", appName);
				return -1;
			}
			break;
		case 'f':
			args->file = optarg;
			break;
		case 'b':
			args->blockSize = atoi(optarg);
			break;
		case 'c':
			args->blockCount = atoi(optarg);
			break;
		case 'r':
			args->reverse = strtol(optarg, NULL, 0);
			args->reverseFlag = 1;
			break;
		case 'x':
			args->bat_flag[0] = 0x69;
			args->bat_flag[1] = 0xaa;
			args->bat_flag[2] = 0x55;
			break;
		case 'h':
		default:
			usage();
			exit(1);
		}
	}

	/* Export no detect args */
	if (optind < argc) {
		int i;
		for (i = optind; i < argc; i++)
			printf("%s: ignoring extra argument -- %s\n", appName, argv[i]);
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct nv_area_wr *area;
	char *fill_buf;
	int out_fd;
	int tailSize;
	ssize_t wSize;
	ssize_t count;
	int err= 0;

	/* Get base name */
	appName = basename(argv[0]);

	/* Parse args */
	memset(&args, 0, sizeof(args));
	if (parse_argument(&args, argc, argv) < 0) {
		usage();
		return -1;
	}

	if (!args.type || !args.file) {
		printf("%s. version or output-file is (null)\n", appName);
		return -1;
	}

	tailSize = 0;
	if (args.blockSize * args.blockCount > sizeof(struct nv_area_wr) * NV_NUMBERS)
		tailSize = args.blockSize * args.blockCount - sizeof(struct nv_area_wr) * NV_NUMBERS;

	/* Default reverse value is 0xff */
	if (!args.reverseFlag)
		args.reverse = 0xff;

	printf("Gen NV image\n\n");

	if (args.type == VER_FLOAT)
		printf("Target version:\t\t%.5f\n", args.ver_f);
	else if (args.type == VER_INT)
		printf("Target version:\t\t%u\n", args.ver_i);
	printf("Output file:\t\t%s\n", args.file);

	if (args.blockSize && args.blockCount) {
		printf("Block size:\t\t%d\n", args.blockSize);
		printf("Block count:\t\t%d\n", args.blockCount);
		printf("Fill reverse:\t\t%#x\n", args.reverse);
	}

	area = malloc(sizeof(struct nv_area_wr) * NV_NUMBERS);
	if (!area) {
		printf("%s. Alloc NV area structure: %s\n", appName, strerror(errno));
		return -1;
	}
	memset(area, 0, sizeof(struct nv_area_wr) * NV_NUMBERS);

	if (args.type == VER_FLOAT)
		area[NV_NUMBERS - 1].current_version_f = args.ver_f;
	else if (args.type == VER_INT)
		area[NV_NUMBERS - 1].current_version_i = args.ver_i;

	area[NV_NUMBERS - 1].resever[0] = args.bat_flag[0];
	area[NV_NUMBERS - 1].resever[1] = args.bat_flag[1];
	area[NV_NUMBERS - 1].resever[2] = args.bat_flag[2];

	fill_buf = malloc(args.blockSize);
	if (!fill_buf) {
		printf("%s. Alloc fill buffer: %s\n", appName, strerror(errno));
		err = -1;
		goto err_alloc_fill;
	}
	memset(fill_buf, args.reverse, args.blockSize);

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	out_fd = open(args.file, O_RDWR | O_CREAT | O_TRUNC, mode);
	if (out_fd < 0) {
		printf("%s. Open %s: %s\n", appName, args.file, strerror(errno));
		err = -1;
		goto err_output_file;
	}

	wSize = write(out_fd, area, sizeof(struct nv_area_wr) * NV_NUMBERS);
	if (wSize < 0) {
		printf("%s. write NV area to output: %s\n", appName, strerror(errno));
		err = -1;
		goto err_write_area;
	}
	count = wSize;

	while (tailSize) {
		int size = tailSize < args.blockSize ?
			tailSize : args.blockSize;
		wSize = write(out_fd, fill_buf, size);
		if (wSize < 0) {
			printf("%s. write tail reverse: %s\n", appName, strerror(errno));
			err = -1;
			goto err_write_fill;
		}

		tailSize -= size;
		count += wSize;
	}

	printf("\nFinal write:\t\t%d\n", count);

err_write_fill:
err_write_area:
	close(out_fd);

err_output_file:
	free(fill_buf);

err_alloc_fill:
	free(area);

	return err;
}
