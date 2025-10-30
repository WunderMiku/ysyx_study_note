#include "npc.h"

static void initMem() {
    M = (vluint32_t*)calloc(MEM_SIZE, sizeof(vluint32_t));
    if (!M) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
}
void loadFile(int argc, char** argv) {
	initMem();
	if (argc < 3) {
		printf(COLOR_RED "Missing file path!\n" COLOR_NONE);
		for(int i = 0; i < argc; i++) {printf("%s", argv[i]);}
		exit(1);
	}
	FILE *file = fopen(argv[2], "rb");
	if (file) {
		// 计算文件大小
		fseek(file, 0, SEEK_END);
		long file_size = ftell(file);
		rewind(file);
		
		// 确保不会超出M数组的大小
		size_t words_to_read = file_size / sizeof(uint32_t);
		if (words_to_read > MEM_SIZE) {
			printf("文件过大，无法加载到内存中,你需要%zu\n", words_to_read);
			exit(1);
		}
		
		// 读取数据到M数组
		size_t read_words = fread(M, sizeof(uint32_t), words_to_read, file);
		fclose(file);
		
		printf(COLOR_GREEN "成功加载 %ld 字节 (%zu 个32位字) 到内存\n" COLOR_NONE , read_words * sizeof(uint32_t), read_words);
	} else {
		printf(COLOR_RED "无法打开文件: %s\n" COLOR_NONE , argv[2]);
		exit(1);
	}
}